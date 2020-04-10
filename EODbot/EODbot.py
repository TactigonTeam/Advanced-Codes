#!/usr/bin/env python
# EODbot UDP to 2 BLE device relay code
# If the serial port don't work run the following command:
# sudo systemctl stop serial-getty@ttyS0.service
# sudo systemctl disable serial-getty@ttyS0.service

import os
import sys
import time
import socket
import serial

# little delay to settle things down after bootup
time.sleep(5)

UDP_IP = '192.168.4.1'
UDP_PORT = 5000

udp = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
udp.bind((UDP_IP, UDP_PORT))
COM = serial.Serial('/dev/ttyS0', 9600)

print('Received data:\n\n')
while True:
    try:
        (data, addr) = udp.recvfrom(32)
        if(data[0] == 'A'):
            print('CINGO: {0}'.format(data))
            COM.write(data)
        else:
            print('ARM: {0}'.format(data))
            COM.write(data)
    except KeyboardInterrupt:
        udp.close()
