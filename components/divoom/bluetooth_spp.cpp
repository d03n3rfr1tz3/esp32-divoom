
#include "bluetooth_spp.h"

#include <algorithm>

#include "platform.h"

#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"
#include "esp_gap_bt_api.h"

// arduino releases the bt controller memory at boot unless a bt library claims it
#if __has_include("esp32-hal-alloc-bt-classic-mem.h")
    #include "esp32-hal-alloc-bt-classic-mem.h"
#else
    extern "C" bool btInUse() { return true; }
#endif

#define SPP_RUNNING      0x01
#define SPP_CONNECTED    0x02
#define SPP_CONGESTED    0x04
#define SPP_DISCONNECTED 0x08
#define SPP_CLOSED       0x10

#define BT_DISCOVERY_COMPLETED 0x02
#define BT_DISCOVERY_CANCEL_TIMEOUT 1000

#define SPP_TX_QUEUE_SIZE     32
#define SPP_TX_QUEUE_TIMEOUT  1000
#define SPP_TX_DONE_TIMEOUT   1000
#define SPP_CONGESTED_TIMEOUT 1000
#define SPP_READY_TIMEOUT     10000
#define SPP_TX_MAX            330

typedef struct {
    size_t len;
    uint8_t data[];
} spp_packet_t;

static uint32_t sppClient = 0;
static bool secondConnectionAttempt = false;
static esp_spp_cb_t sppCallback = NULL;

static EventGroupHandle_t sppEvents = NULL;
static EventGroupHandle_t gapEvents = NULL;
static QueueHandle_t txQueue = NULL;
static SemaphoreHandle_t txDone = NULL;

static uint8_t txBuffer[SPP_TX_MAX];
static size_t txLength = 0;

static std::vector<BluetoothDevice> devices;

/**
 * sends the collected tx buffer and waits for its acknowledgement
*/
static bool txSend(void) {
    if ((xEventGroupWaitBits(sppEvents, SPP_CONGESTED, pdFALSE, pdTRUE, SPP_CONGESTED_TIMEOUT / portTICK_PERIOD_MS) & SPP_CONGESTED) == 0) return false;

    if (!sppClient) {
        txLength = 0;
        return false;
    }

    if (esp_spp_write(sppClient, txLength, txBuffer) != ESP_OK) return false;
    txLength = 0;

    return xSemaphoreTake(txDone, SPP_TX_DONE_TIMEOUT / portTICK_PERIOD_MS) == pdTRUE;
}

/**
 * background task that coalesces queued packets into the tx buffer
*/
static void txTask(void *parameter) {
    spp_packet_t *packet;
    for (;;) {
        if (xQueueReceive(txQueue, &packet, portMAX_DELAY) != pdTRUE || !packet) continue;

        const uint8_t *data = packet->data;
        size_t length = packet->len;
        while (length) {
            size_t chunk = std::min(length, (size_t)(SPP_TX_MAX - txLength));
            memcpy(txBuffer + txLength, data, chunk);
            txLength += chunk;
            data += chunk;
            length -= chunk;

            if (txLength == SPP_TX_MAX || (length == 0 && uxQueueMessagesWaiting(txQueue) == 0)) {
                if (!txSend()) break;
            }
        }

        free(packet);
    }
}

/**
 * callback for spp events, tracks the connection before passing them on
*/
static void sppEvent(esp_spp_cb_event_t event, esp_spp_cb_param_t *param) {
    switch (event) {
        case ESP_SPP_INIT_EVT:
            esp_bt_gap_set_scan_mode(ESP_BT_CONNECTABLE, ESP_BT_GENERAL_DISCOVERABLE);
            xEventGroupSetBits(sppEvents, SPP_RUNNING);
            break;
        case ESP_SPP_OPEN_EVT:
            if (!sppClient) {
                sppClient = param->open.handle;
            } else {
                secondConnectionAttempt = true;
                esp_spp_disconnect(param->open.handle);
            }
            xEventGroupClearBits(sppEvents, SPP_DISCONNECTED);
            xEventGroupSetBits(sppEvents, SPP_CONNECTED | SPP_CONGESTED);
            break;
        case ESP_SPP_CLOSE_EVT:
            if (param->close.async || param->close.status == ESP_SPP_SUCCESS) {
                if (secondConnectionAttempt) {
                    secondConnectionAttempt = false;
                } else {
                    sppClient = 0;
                    xEventGroupSetBits(sppEvents, SPP_DISCONNECTED | SPP_CONGESTED | SPP_CLOSED);
                    xEventGroupClearBits(sppEvents, SPP_CONNECTED);
                }
            }
            break;
        case ESP_SPP_CONG_EVT:
            if (param->cong.cong) xEventGroupClearBits(sppEvents, SPP_CONGESTED);
            else xEventGroupSetBits(sppEvents, SPP_CONGESTED);
            break;
        case ESP_SPP_WRITE_EVT:
            if (param->write.status == ESP_SPP_SUCCESS && param->write.cong) xEventGroupClearBits(sppEvents, SPP_CONGESTED);
            xSemaphoreGive(txDone);
            break;
        default:
            break;
    }

    if (sppCallback) sppCallback(event, param);
}

/**
 * callback for gap events, collects discovery results and answers legacy pairing
*/
static void gapEvent(esp_bt_gap_cb_event_t event, esp_bt_gap_cb_param_t *param) {
    switch (event) {
        case ESP_BT_GAP_DISC_RES_EVT: {
            std::string name;
            for (int i = 0; i < param->disc_res.num_prop; i++) {
                esp_bt_gap_dev_prop_t *prop = &param->disc_res.prop[i];
                if (prop->type == ESP_BT_GAP_DEV_PROP_BDNAME) {
                    name.assign((const char *)prop->val, strnlen((const char *)prop->val, prop->len));
                } else if (prop->type == ESP_BT_GAP_DEV_PROP_EIR && name.empty()) {
                    uint8_t length = 0;
                    uint8_t *eir = esp_bt_gap_resolve_eir_data((uint8_t *)prop->val, ESP_BT_EIR_TYPE_CMPL_LOCAL_NAME, &length);
                    if (!eir) eir = esp_bt_gap_resolve_eir_data((uint8_t *)prop->val, ESP_BT_EIR_TYPE_SHORT_LOCAL_NAME, &length);
                    if (eir) name.assign((const char *)eir, length);
                }
            }

            auto known = std::find_if(devices.begin(), devices.end(), [param](const BluetoothDevice &device) {
                return memcmp(device.address.getNative(), param->disc_res.bda, ESP_BD_ADDR_LEN) == 0;
            });
            if (known == devices.end()) devices.push_back({ BTAddress(param->disc_res.bda), name });
            else if (known->name.empty()) known->name = name;
            break;
        }
        case ESP_BT_GAP_DISC_STATE_CHANGED_EVT:
            if (param->disc_st_chg.state == ESP_BT_GAP_DISCOVERY_STOPPED) xEventGroupSetBits(gapEvents, BT_DISCOVERY_COMPLETED);
            else xEventGroupClearBits(gapEvents, BT_DISCOVERY_COMPLETED);
            break;
        case ESP_BT_GAP_PIN_REQ_EVT: {
            esp_bt_pin_code_t code;
            if (param->pin_req.min_16_digit) {
                memset(code, '0', ESP_BT_PIN_CODE_LEN);
                esp_bt_gap_pin_reply(param->pin_req.bda, true, ESP_BT_PIN_CODE_LEN, code);
            } else {
                memcpy(code, "1234", 4);
                esp_bt_gap_pin_reply(param->pin_req.bda, true, 4, code);
            }
            break;
        }
        default:
            break;
    }
}

/**
 * starts the controller, bluedroid and spp
*/
bool BluetoothSpp::begin(const char *name) {
    sppEvents = xEventGroupCreate();
    gapEvents = xEventGroupCreate();
    txQueue = xQueueCreate(SPP_TX_QUEUE_SIZE, sizeof(spp_packet_t *));
    txDone = xSemaphoreCreateBinary();
    if (!sppEvents || !gapEvents || !txQueue || !txDone) return false;
    xEventGroupSetBits(sppEvents, SPP_CONGESTED | SPP_DISCONNECTED | SPP_CLOSED);

    if (xTaskCreatePinnedToCore(txTask, "spp_tx", 4096, NULL, configMAX_PRIORITIES - 1, NULL, 0) != pdPASS) return false;

    esp_bt_controller_config_t config = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    if (esp_bt_controller_get_status() == ESP_BT_CONTROLLER_STATUS_IDLE && esp_bt_controller_init(&config) != ESP_OK) return false;
    if (esp_bt_controller_get_status() == ESP_BT_CONTROLLER_STATUS_INITED && esp_bt_controller_enable((esp_bt_mode_t)config.mode) != ESP_OK) return false;

    if (esp_bluedroid_get_status() == ESP_BLUEDROID_STATUS_UNINITIALIZED && esp_bluedroid_init() != ESP_OK) return false;
    if (esp_bluedroid_get_status() != ESP_BLUEDROID_STATUS_ENABLED && esp_bluedroid_enable() != ESP_OK) return false;

    if (esp_bt_gap_register_callback(gapEvent) != ESP_OK) return false;
    if (esp_spp_register_callback(sppEvent) != ESP_OK) return false;
    if (DIVOOM_BT_SPP_INIT() != ESP_OK) return false;

    DIVOOM_BT_SET_NAME(name);

    esp_bt_cod_t cod = {};
    cod.major = 0b00001;
    cod.minor = 0b000100;
    cod.service = 0b00000010110;
    return esp_bt_gap_set_cod(cod, ESP_BT_INIT_COD) == ESP_OK;
}

/**
 * connects to the given address and channel
*/
bool BluetoothSpp::connect(const BTAddress &address, uint16_t channel) {
    if ((xEventGroupWaitBits(sppEvents, SPP_RUNNING, pdFALSE, pdTRUE, SPP_READY_TIMEOUT / portTICK_PERIOD_MS) & SPP_RUNNING) == 0) return false;
    disconnect();

    esp_bd_addr_t remote;
    memcpy(remote, address.getNative(), ESP_BD_ADDR_LEN);

    xEventGroupClearBits(sppEvents, SPP_CLOSED);
    if (esp_spp_connect(ESP_SPP_SEC_ENCRYPT | ESP_SPP_SEC_AUTHENTICATE, ESP_SPP_ROLE_MASTER, channel, remote) != ESP_OK) return false;

    return connected(SPP_READY_TIMEOUT);
}

/**
 * waits until connected or closed
*/
bool BluetoothSpp::connected(uint32_t timeout) {
    EventBits_t bits = xEventGroupWaitBits(sppEvents, SPP_CONNECTED | SPP_CLOSED, pdFALSE, pdFALSE, timeout / portTICK_PERIOD_MS);
    return (bits & SPP_CONNECTED) != 0;
}

/**
 * flushes the tx queue and disconnects
*/
bool BluetoothSpp::disconnect(void) {
    if (!sppClient) return false;

    while (uxQueueMessagesWaiting(txQueue) > 0) delay(2);

    if (esp_spp_disconnect(sppClient) != ESP_OK) return false;
    return (xEventGroupWaitBits(sppEvents, SPP_DISCONNECTED, pdFALSE, pdTRUE, SPP_READY_TIMEOUT / portTICK_PERIOD_MS) & SPP_DISCONNECTED) != 0;
}

/**
 * sets a fixed pin for legacy pairing, an empty pin restores the variable one
*/
void BluetoothSpp::setPin(const char *pin) {
    esp_bt_pin_code_t code = {};
    size_t length = strnlen(pin, ESP_BT_PIN_CODE_LEN);
    memcpy(code, pin, length);

    esp_bt_gap_set_pin(length ? ESP_BT_PIN_TYPE_FIXED : ESP_BT_PIN_TYPE_VARIABLE, length, code);
}

/**
 * runs an inquiry and returns the found devices
*/
std::vector<BluetoothDevice> BluetoothSpp::discover(int timeout) {
    devices.clear();
    xEventGroupClearBits(gapEvents, BT_DISCOVERY_COMPLETED);

    if (esp_bt_gap_start_discovery(ESP_BT_INQ_MODE_GENERAL_INQUIRY, timeout / 1280, 0) == ESP_OK) {
        if ((xEventGroupWaitBits(gapEvents, BT_DISCOVERY_COMPLETED, pdFALSE, pdTRUE, timeout / portTICK_PERIOD_MS) & BT_DISCOVERY_COMPLETED) == 0) {
            esp_bt_gap_cancel_discovery();
            xEventGroupWaitBits(gapEvents, BT_DISCOVERY_COMPLETED, pdFALSE, pdTRUE, BT_DISCOVERY_CANCEL_TIMEOUT / portTICK_PERIOD_MS);
        }
    }

    return devices;
}

/**
 * cancels a running inquiry
*/
void BluetoothSpp::discoverStop(void) {
    esp_bt_gap_cancel_discovery();
}

/**
 * queues data for the tx task
*/
size_t BluetoothSpp::write(const uint8_t *buffer, size_t size) {
    if (!sppClient || !size) return 0;

    spp_packet_t *packet = (spp_packet_t *)malloc(sizeof(spp_packet_t) + size);
    if (!packet) return 0;
    packet->len = size;
    memcpy(packet->data, buffer, size);

    if (xQueueSend(txQueue, &packet, SPP_TX_QUEUE_TIMEOUT / portTICK_PERIOD_MS) != pdPASS) {
        free(packet);
        return 0;
    }

    return size;
}

/**
 * registers the callback for spp events
*/
void BluetoothSpp::onEvent(esp_spp_cb_t callback) {
    sppCallback = callback;
}
