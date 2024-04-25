# esp32-divoom
[![Compile Project](https://github.com/d03n3rfr1tz3/esp32-divoom/actions/workflows/build.yml/badge.svg)](https://github.com/d03n3rfr1tz3/esp32-divoom/actions/workflows/build.yml)
[![GitHub Release](https://img.shields.io/github/v/release/d03n3rfr1tz3/esp32-divoom?include_prereleases)](https://github.com/d03n3rfr1tz3/esp32-divoom/releases/latest)

**Divoom Proxy for ESP32**

Allows you to send commands to your Divoom device through various protocols. It works as a companion for my [Home Assistant integration](https://github.com/d03n3rfr1tz3/hass-divoom),
but you can also use it as standalone communicator for your Divoom devices. Currently the following input protocols are either implemented or planned. You can find more information
in the documentation below.

* **Serial**
* **TCP**
* **MQTT** (TODO)

## Installation

This firmware is a PlatformIO project. Until I can find and prepare a more easy way for you to get started, you have to just download the source code and build and upload it to an ESP32 via VS Code yourself.
In the future I might use [ESP Web Tools](https://esphome.github.io/esp-web-tools/) for an easier installation process.

* Download the repository. If you know git, a clone is fine. If not, just download https://github.com/d03n3rfr1tz3/esp32-divoom/archive/main.zip to get the most recent code in a ZIP file.
* Copy the corresponding content of the ZIP file into a directory of your choice
* Open the directory with PlatformIO via VS Code
* Connect your ESP32 with a working USB Data Cable
* Press `Upload` in the PlatformIO Project Tasks
* ...
* Profit

## Documentation

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

You can also control your Divoom devices via MQTT. This is variant you probably will use, when you are going for the standalone mode. The actual commands are similar to the Serial input.

TODO
