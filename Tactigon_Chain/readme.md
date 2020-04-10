# Tactigon_Chain

The aim of this project is showing on screen the real time representation of a chain of 3 Tactigon boards.
This would show the potentiality of a real time monitoring of orientation of complex system like robotic arm.

Take a look at our [video demo](https://github.com/TactigonTeam/Advanced-Codes/blob/master/Tactigon_Chain/Tactigon_Chain_Demo.m4v).

## Structure
From the hardware point of view the project is composed by:
* 3 Tactigon boards
* a raspberry PI 3

Raspberry has CENTRAL role on BLE interface, continuously scan searching for devices named “Test1” , “Test2”, “Test3”.
Once it finds them it performs two step of calibration and than show on screen a 2D real time representation of the Tactigon chain.

On Tactigon side all three boards run the same script [TactigonSketch.ino](https://github.com/TactigonTeam/Advanced-Codes/blob/master/Tactigon_Chain/TactigonSketch/TactigonSketch.ino), just device name changes at line 47.
## Scripts

Python script is available in [Multicast-demo](https://github.com/TactigonTeam/Advanced-Codes/tree/master/Tactigon_Chain/Multicast-demo) folder. In order to run it just type at the command line prompt:
```
sudo python multicast_ble_q.py
```
Please take note that super user rights are required in order to get access to bluetooth interface.

### Prerequisites

* Python 2.7
* [BlueZ library](https://learn.adafruit.com/install-bluez-on-the-raspberry-pi/installation) - BlueZ library with support for BLE
* [Bluepy library 1.2.0](https://github.com/IanHarvey/bluepy) - Use v1.2.0, newer versions causes connection troubles... 

## Built with

* [Procesing 3.5.3](http://download.processing.org/processing-3.5.3-windows64.zip) - Win64
* [Procesing 3.5.3](http://download.processing.org/processing-3.5.3-windows32.zip) - Win32
