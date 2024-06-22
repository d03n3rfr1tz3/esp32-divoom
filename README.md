# esp32-divoom
[![Compile Project](https://github.com/d03n3rfr1tz3/esp32-divoom/actions/workflows/build.yml/badge.svg)](https://github.com/d03n3rfr1tz3/esp32-divoom/actions/workflows/build.yml)
[![GitHub Release](https://img.shields.io/github/v/release/d03n3rfr1tz3/esp32-divoom?include_prereleases)](https://github.com/d03n3rfr1tz3/esp32-divoom/releases/latest)

**Divoom Proxy for ESP32**

Allows you to send commands to your Divoom device through various protocols. It works as a companion for my [Home Assistant integration](https://github.com/d03n3rfr1tz3/hass-divoom),
but you can also use it as standalone communicator for your Divoom devices. Currently the following input protocols are implemented. You can find more information in the documentation below.

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
      - [MODE alarm](#mode-alarm)
      - [MODE brightness](#mode-brightness)
      - [MODE clock](#mode-clock)
      - [MODE countdown](#mode-countdown)
      - [MODE datetime](#mode-datetime)
      - [MODE design](#mode-design)
      - [MODE effects](#mode-effects)
      - [MODE game](#mode-game)
      - [MODE gamecontrol](#mode-gamecontrol)
      - [MODE keyboard](#mode-keyboard)
      - [MODE light](#mode-light)
      - [MODE lyrics](#mode-lyrics)
      - [MODE memorial](#mode-memorial)
      - [MODE noise](#mode-noise)
      - [MODE playstate](#mode-playstate)
      - [MODE radio](#mode-radio)
      - [MODE scoreboard](#mode-scoreboard)
      - [MODE sleep](#mode-sleep)
      - [MODE timer](#mode-timer)
      - [MODE visualization](#mode-visualization)
      - [MODE volume](#mode-volume)
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

Commands should be sent to the topic `MQTT_TOPIC/command`. For the `MQTT_TOPIC` part, you have to look into your `config_local.h`. Default is `divoom/`, which makes the command topic `divoom/command`.

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

#### MODE alarm
Sets an alarm. You might have to experiment with the options your Divoom device supports and what it actually changes. Unsupported values will be ignored or if possible directly zeroed by this component, to prevent strange behavior.

`MODE alarm [number] [time] [weekdays] [alarmmode] [triggermode] [frequency] [volume]`
| Parameter | Description |
| ---       | ---         |
| `number`  | The concrete slot for the alarm. For the actual amount of slots you might have to look into the phone app. |
| `time`    | The concrete time for when the alarm should happen in the format `mm:ss`. |
| `weekdays` | The list of `0` and `1` for each weekday starting with sunday. Example: `1000001` for weekend only alarm. |
| `alarmmode` | The alarm mode. Look into your phone app for what is supported by your Divoom device. |
| `triggermode` | The trigger mode. Look into your phone app for what is supported by your Divoom device. |
| `frequency` | The radio frequency to set. |
| `volume`  | The volume of the alarm. |

```
MODE alarm 1 07:30 0111110 0 0 85.1 100
```

#### MODE brightness
Sets the brightness.

`MODE brightness [value]`
| Parameter | Description |
| ---       | ---         |
| `value`   | The brightness value between 0 and 100. |

```
MODE brightness 100
```

#### MODE clock
Shows the clock channel.

`MODE clock [clock] [twentyfour] [weather] [temp] [calendar] [color] [hot]`
| Parameter | Description |
| ---       | ---         |
| `clock`   | The style of the clock. Accepts a number between 0 and 9. <br/> `0` = Fullscreen, `1` = Rainbow, `2` = Boxed, `3` = Analog square, <br/> `4` = Fullscreen negative, `5` = Analog round, `6` = Widescreen |
| `twentyfour` | Changes between 12h or 24h format. <br/> `0` = 12h, `1` = 24h. Defaults to 24h. Doesn't actually change the current time. |
| `weather` | Actives or deactivates showing the weather with `0` or `1`. |
| `temp`    | Actives or deactivates showing the temperature with `0` or `1`. |
| `calendar` | Actives or deactivates showing the calendar date with `0` or `1`. |
| `color`   | The color of the clock in the typical RGB HEX format. Example: `FF0000` for red. |
| `hot`     | Actives or deactivates showing the slideshow of the best images with `0` or `1`, which is right next to the other boolean-like buttons in the app, but a completely separate command in the protocol |

```
MODE clock 1 1 1 1 1 FF0000 1
```

#### MODE countdown
Shows the countdown tool.

`MODE countdown [value] [countdown]`
| Parameter | Description |
| ---       | ---         |
| `value`   | Controls the start/stop state. <br/> `0` = stop, `1` = start |
| `countdown` | The concrete countdown in the format `mm:ss`. |

```
MODE countdown 1 02:30
```

#### MODE datetime
Sets the datetime.

`MODE datetime [date] [time]`
| Parameter | Description |
| ---       | ---         |
| `date`    | The date in typical ISO format. |
| `time`    | The time in typical ISO format. |

```
MODE datetime 2020-12-24 20:15:00
```

#### MODE design
Shows the design channel.

`MODE design [value]`
| Parameter | Description |
| ---       | ---         |
| `value`   | The number of the concrete design. Ranging from 0-2 you can specify the design 1-3. |

```
MODE design 0
```

#### MODE effects
Shows the effects channel.

`MODE effects [value]`
| Parameter | Description |
| ---       | ---         |
| `value`   | The number of the concrete effect. Might differ for some Divoom devices. Look into your phone app and count them. |

```
MODE effects 1
```

#### MODE game
Shows a game. It is theoretically possible to open games, that are not shown in your phone app, but they might not work very well.

`MODE game [value]`
| Parameter | Description |
| ---       | ---         |
| `value` | The number of the concrete game. Depending on your device you may have different amount of games. Look into your phone app and count them. |

```
MODE game 1
```

#### MODE gamecontrol
Sends controlling commands to the currently open game.

`MODE gamecontrol [value]`
| Parameter | Description |
| ---       | ---         |
| `value` | `0` or `go` = go, <br/> `1` or `left` = left, <br/> `2` or `right` = right, <br/> `3` or `up` = up, <br/> `4` or `bottom` = bottom, <br/> `5` or `ok` = ok |

```
MODE gamecontrol 0
```

#### MODE keyboard
Controls the keyboard LEDs specifically on the Ditoo.

`MODE keyboard [value]`
| Parameter | Description |
| ---       | ---         |
| `value`   | `0` = toggle keyboard light on/off, `1` = switch to next effect, `-1` = switch to previous effect |

```
MODE keyboard 0
```

#### MODE light
Shows the light channel.

`MODE light [brightness] [color] [power]`
| Parameter | Description |
| ---       | ---         |
| `brightness` | The brightness value between 0 and 100. |
| `color`   | The color of the clock in the typical RGB HEX format. Example: `FF0000` for red. |
| `power`   | Activates or deactivates powering the LED panel. |

```
MODE light 100 FF0000 1
```

#### MODE lyrics
Shows the lyrics channel. Might not be supported by every Divoom device.

`MODE lyrics`
| Parameter | Description |
| ---       | ---         |

```
MODE lyrics
```

#### MODE memorial
Sets a memorial (reminder).

`MODE memorial [number] [date] [time] [text]`
| Parameter | Description |
| ---       | ---         |
| `number`  | The concrete slot for the memorial. For the actual amount of slots you might have to look into the phone app. |
| `date`    | The date in typical ISO format. The year will be ignored. |
| `time`    | The time in typical ISO format. The seconds will be ignored. |
| `text`    | Specifies the name of your memorial, as it will appear in the phone app (default: ESP32). Limited to 16 characters. |

```
MODE memorial 1 2020-12-31 59:59:59 Happy New Year!
```

#### MODE noise
Shows the noise meter.

`MODE noise [value]`
| Parameter | Description |
| ---       | ---         |
| `value`   | Controls the start/stop state. <br/> `0` = stop, `1` = start |

```
MODE noise 1
```

#### MODE playstate
Sets the playstate for the currently played music. Only supported by Divoom devices with audio features.

`MODE playstate [value]`
| Parameter | Description |
| ---       | ---         |
| `value`   | `0` = pause, `1` = play |

```
MODE playstate 0
```

#### MODE radio
Shows and plays the radio channel. Only supported by Divoom devices with the radio feature.

`MODE radio [value] [frequency]`
| Parameter | Description |
| ---       | ---         |
| `value` | Controls the on/off state. <br/> `0` = off, `1` = on |
| `frequency` | The radio frequency to set. |

```
MODE radio 1 85.1
```

#### MODE scoreboard
Shows the scoreboard channel or tool.

`MODE scoreboard [version] [player1] [player2]`
| Parameter | Description |
| ---       | ---         |
| `version` | The version to use. Depending on the age of your Divoom device, you might have to use `0` or `1`. If you used `0` and it opens the lyrics channel, you need to use `1`. <br/> `0` = Opens the scoreboard channel. `1` = Opens the scoreboard tool. |
| `player1` | The score of player one to show. |
| `player2` | The score of player two to show. |

```
MODE scoreboard 0 1 2
```

#### MODE sleep
Shows the sleep mode, which plays soothing sounds, optionally with a timer and light.

| Parameter    | Description |
| ---          | ---         |
| `value`      | Controls the start/stop state. <br/> `0` = stop, `1` = start |
| `time`       | The time in minutes after which to stop the sleep mode. |
| `sleepmode`  | The sound effect to play. Check in the app how many options are available. Accepts a number. |
| `frequency`  | The radio frequency to set. |
| `volume`     | The volume value between 0 and 100. |
| `color`      | The color of the display in the typical RGB HEX format. Example: `FF0000` for red. |
| `brightness` | The brightness value between 0 and 100. |

```
MODE sleep 1 30 4 85.1 100 FF0000 100
```

#### MODE timer
Shows the timer tool.

`MODE timer [value]`
| Parameter | Description |
| ---       | ---         |
| `value`   | Controls the start/stop state. <br/> `0` = stop, `1` = start |

```
MODE timer 1
```

#### MODE visualization
Shows the visualization channel.

`MODE visualization [value]`
| Parameter | Description |
| ---       | ---         |
| `value`   | The number of the concrete visualization. Might differ for some Divoom devices. Look into your phone app and count them. |

```
MODE visualization 1
```

#### MODE volume
Sets the volume. Only supported by Divoom devices with audio features.

`MODE volume [value]`
| Parameter | Description |
| ---       | ---         |
| `value`   | The volume value between 0 and 100. |

```
MODE volume 100
```

#### MODE weather
Sets the weather.

`MODE weather [temperature] [weather]`
| Parameter | Description |
| ---       | ---         |
| `temperature` | The temperature in degree including the temperature type for celsius or fahrenheit. |
| `weather` | `1` = clear, `3` = cloudy sky, `5` = thunderstorm, `6` = rain, `8` = snow, `9` = fog |

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
