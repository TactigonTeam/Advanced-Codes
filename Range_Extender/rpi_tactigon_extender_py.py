#!/usr/bin/env python
#UDP SERVER CODE
#To run this script at boot you have te add the following to the /etc/rc.local file in sudo mode
#sudo python /path_to_this_script/script_name.py
#before the exit 0 directive
import os
import sys
import socket
import serial
import time

UDP_PORT = 5000
UDP_IP = '192.168.4.1' #IP address of the machine to whick this script will run
#small delay to ensure that everithing started up
time.sleep(10)

port = serial.Serial("/dev/ttyGS0", baudrate=115200, timeout=0.1)

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind((UDP_IP, UDP_PORT))

while ((port.in_waiting) <= 0):
    data, addr = sock.recvfrom(1024) #store received data
    #print(data) #for debug
    port.write(data + '\n') #write received data from UDP to the emulated serial port
#if the input buffer of the serial port is NOT empty that means that the shutdown command has been received from the PC
os.system("shutdown now -h")