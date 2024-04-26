# esp32-divoom
[![Compile Project](https://github.com/d03n3rfr1tz3/esp32-divoom/actions/workflows/build.yml/badge.svg)](https://github.com/d03n3rfr1tz3/esp32-divoom/actions/workflows/build.yml)
[![GitHub Release](https://img.shields.io/github/v/release/d03n3rfr1tz3/esp32-divoom?include_prereleases)](https://github.com/d03n3rfr1tz3/esp32-divoom/releases/latest)

**Divoom Proxy for ESP32**

Allows you to send commands to your Divoom device through various protocols. It works as a companion for my [Home Assistant integration](https://github.com/d03n3rfr1tz3/hass-divoom),
but you can also use it as standalone communicator for your Divoom devices. Currently the following input protocols are either implemented or planned. You can find more information
in the documentation below.

* **Serial**
* **TCP**
* **MQTT**

## Table of Contents
* [Requirements](#requirements)
* [Installation](#installation)
  + [Easy Installation](#easy-installation)
  + [Manual Installation](#manual-installation)
* [Configuration](#configuration)
  + [Easy Configuration](#easy-configuration)
  + [Manual Configuration](#manual-configuration)
* [Usage](#usage)
  + [Serial](#serial)
  + [TCP](#tcp)
  + [MQTT](#mqtt)
  + [MODE Commands](#mode-commands)
    - [MODE brightness](#mode-brightness)
    - [MODE volume](#mode-volume)
    - [MODE keyboard](#mode-keyboard)
    - [MODE playstate](#mode-playstate)
    - [MODE weather](#mode-weather)
* [Troubleshooting](#troubleshooting)
  + [Cannot connect](#cannot-connect)
  + [GIF does not work](#gif-does-not-work)

## Requirements

This firmware obviously needs an ESP32. Other then that, not much is needed, as the ESP32 already brings WiFi and Bluetooth with it.

## Installation

### Easy Installation

In the future I might use [ESP Web Tools](https://esphome.github.io/esp-web-tools/) for an easier installation process. For now, you have to go for the manual installation.

### Manual Installation

This firmware is a PlatformIO project. Until I can find and prepare a more easy way for you to get started, you have to just download the source code and build and upload it to an ESP32 via VS Code yourself.

* Download the repository. If you know git, a clone is fine. If not, just download https://github.com/d03n3rfr1tz3/esp32-divoom/archive/main.zip to get the most recent code in a ZIP file.
* Copy the corresponding content of the ZIP file into a directory of your choice
* Open the directory with PlatformIO via VS Code
* Connect your ESP32 with a working USB Data Cable
* Press `Upload` in the PlatformIO Project Tasks
* ...
* Profit

## Configuration

### Easy Configuration

Well again, currently there is no easy way, but that might change in the future.

### Manual Configuration

Currently you need to configure the firmware directly in your own `config_local.h` before flashing. This might change in the future.

The default configuration `config.h` has a lot of empty values you very likely want to fill. To not run into problems with later updates,
I recommend you to create a `config_local.h` with your own values. Here is an example:

```
#ifndef _CONFIG_LOCAL_H
    #define _CONFIG_LOCAL_H
    
    #undef WIFISSID1
    #undef WIFIPASS1
    #define WIFISSID1        "My WiFi 1"
    #define WIFIPASS1        "MySuperSecretPassword1"

    #undef WIFISSID2
    #undef WIFIPASS2
    #define WIFISSID2        "My WiFi 2"
    #define WIFIPASS2        "MySuperSecretPassword2"

    #undef MQTT_HOST
    #undef MQTT_USER
    #undef MQTT_PASS
    #define MQTT_HOST        "192.168.0.2"
    #define MQTT_USER        "mqtt"
    #define MQTT_PASS        "MySecretPassword"
#endif
```

Notice the undefining of each value before defining it with my own value. That way you don't get ugly warnings from the compiler later. 😉

## Usage

### Serial

You can control your Divoom devices via Serial. Obviously this is more for debugging purposes, but might also be helpful for a quick test. If you press `Monitor` in the PlatformIO Project Tasks, you can
send some commands directly to your ESP32. Just prepare them in Notepad and then copy them into the Terminal (you will not see what you typed/copied) and press Enter. Depending on the actual command,
the ESP32 will then send commands to your Divoom device and also give you some information or received packets back. Here are some examples:

This command connects to your Divoom device with the MAC address `01:12:23:45:56:67` and on port `1`.
````
CONNECT 01:12:23:45:56:67 1
````

This command sends raw bytes to your connected Divoom device. The raw bytes represent the `brightness` command. The first one sets the brightness to 50% and the second one to 100%.
````
SEND 01 04 00 74 32 AA 00 02
SEND 01 04 00 74 64 DC 00 02
````

This command disconnect from your Divoom device.
````
DISCONNECT 01:12:23:45:56:67
````

This command sets the brightness to 100% of your connected Divoom device. As you can see, this is a more human readable format. You can find a full list of MODE commands here: [MODE commands](#mode-commands)
````
MODE brightness 100
````

### TCP

You can also control your Divoom devices via TCP. Obviously that is not really meant for user interaction, but for actual communication one a lower level. The most used case for TCP is my own
[Home Assistant integration](https://github.com/d03n3rfr1tz3/hass-divoom), which uses this ESP32 firmware as a Bluetooth Proxy. But maybe you want to develop your own application based on this,
so here are some examples:

This connects to your Divoom device with the MAC address `01:12:23:45:56:67` and on port `1`. The packet starts with a single byte `0x69` hinting this is a connect packet, followed
by the MAC address also in raw bytes as well as the optional port number.
````
690112233445566701
````

This sends raw bytes to your connected Divoom device. The raw bytes represent the `brightness` command. Typical for the Divoom protocol, the packet starts with `0x01` and ends with `0x02`.
The first packet sets the brightness to 50% and the second one to 100%.
````
0104007432AA0002
0104007464DC0002
````

This disconnect from your Divoom device. The packet starts with a single byte `0x96` hinting this is a disconnect packet, followed by the MAC address also in raw bytes.
````
960112233445566701
````

### MQTT

You can also control your Divoom devices via MQTT as well as getting a few states. This is variant you probably will use, when you are going for the standalone mode. The actual commands are similar to the Serial input. Additionally the general state will be published to `MQTT_TOPIC/proxy`, the bluetooth connection state to `MQTT_TOPIC/bluetooth` and advertise bluetooth devices to `MQTT_TOPIC/advertise/[MAC]` while `[MAC]` stands for the actual MAC address like `01:12:23:45:56:67`.

This command connects to your Divoom device with the MAC address `01:12:23:45:56:67` and on port `1`.
````
CONNECT 01:12:23:45:56:67 1
````

This command sends raw bytes to your connected Divoom device. The raw bytes represent the `brightness` command. The first one sets the brightness to 50% and the second one to 100%.
````
SEND 01 04 00 74 32 AA 00 02
SEND 01 04 00 74 64 DC 00 02
````

This command disconnect from your Divoom device.
````
DISCONNECT 01:12:23:45:56:67
````

This command sets the brightness to 100% of your connected Divoom device. As you can see, this is a more human readable format. You can find a full list of MODE commands here: [MODE commands](#mode-commands)
````
MODE brightness 100
````

### MODE Commands

Here you can find a full list of MODE commands that are supported as well as some descriptions and examples. If you are coming from my
[Home Assistant integration](https://github.com/d03n3rfr1tz3/hass-divoom), you probably will notice a few similarities.

#### MODE brightness

`MODE brightness value`
| Parameter | Description |
| ---       | ---         |
| value | The brightness value between 0 and 100. |

```
MODE brightness 100
```

#### MODE keyboard

`MODE keyboard value`
| Parameter | Description |
| ---       | ---         |
| value     | `0` = toggle keyboard light on/off, `1` = switch to next keyboard light mode, `-1` = switch to previous keyboard light mode |

```
MODE keyboard 0
```

#### MODE playstate

`MODE playstate value`
| Parameter | Description |
| ---       | ---         |
| value     | `0` = pause, `1` = play |

```
MODE playstate 0
```

#### MODE volume

`MODE volume value`
| Parameter | Description |
| ---       | ---         |
| value     | The volume value between 0 and 100. |

```
MODE volume 100
```

#### MODE weather

`MODE weather temperature weather`
| Parameter | Description |
| ---       | ---         |
| temperature | The temperature in degree including the temperature type for celsius or fahrenheit. |
| weather | `1` = clear, `3` = cloudy sky, `5` = thunderstorm, `6` = rain, `8` = snow, `9` = fog |

```
MODE weather 25°C 1
```

## Troubleshooting
### Cannot connect
Make sure, that your Phone is not currently connected to your Divoom device, because some don't allow that many connections.

If it seems to connect, but looses connection the moment you use any mode, you might have chosen the wrong port. On Pixoo and other non-audio
devices, it's typically port `1`. But on audio devices, like the Tivoo or Ditoo, it might be port `2`.

### GIF does not work

The most common problem is, that the GIF does not have the correct size or format. The Divoom devices (and to some extend my code) are nitpicky in that case. Strangly enough the Divoom app lets you download GIFs, but these are typically in the size of 320x320 and not fitting your device.
Your GIF needs to be exactly the size of your Divoom screen (*16x16* in case of a Pixoo or similar sized device), *non-interlaced* and with a *global color palette*.

I can recommend resizing and converting your GIFs with GIMP. Of course other software might also work, depending on the export/format options. When resizing a GIF downloaded from the Divoom app with GIMP, you better choose no interpolation to not blur your GIF. When exporting with GIMP, make sure to mark the animation checkbox and don't mark the interlace checkbox. For a few more details and an example look into the following comment: https://github.com/d03n3rfr1tz3/hass-divoom/issues/19#issuecomment-1982059358
