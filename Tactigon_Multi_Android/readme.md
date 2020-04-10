# Tactigon_Multi_Android

The aim of this project is showing on screen via plots of acceleration, environment data and angles of 3 Tskin/Tactigon board.

## Structure
* Tactigon (Arduino) side:
    Three tactigon expose accelerations, angular speed, Euler Angles, Environment data on BLE.
    The sketch is the same, just you must change the BLE name for each Tactigon board in DEMO1, DEMO2, DEMO3

* Android side:
    Processing app: search and connect to DEMO1, DEMO2, DEMO3 and plot accelerations, environment data, angles respectively

### Prerequisites

* Python 2.7
* [BlueZ library](https://learn.adafruit.com/install-bluez-on-the-raspberry-pi/installation) - BlueZ library with support for BLE
* [Bluepy library 1.2.0](https://github.com/IanHarvey/bluepy) - Use v1.2.0, newer versions causes connection troubles... 

## Built with

* [Procesing 3.5.3](http://download.processing.org/processing-3.5.3-windows64.zip) - Win64
* [Procesing 3.5.3](http://download.processing.org/processing-3.5.3-windows32.zip) - Win32
