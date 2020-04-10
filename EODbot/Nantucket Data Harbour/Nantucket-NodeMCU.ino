/*
 Name:		Nantucket-NodeMCU.ino
 Author:	Luca Seva'
 Description:	The safe harbour of Nantucket-Mega once that receives the data from the BLE modules packs those information in buffers that will be sent via UDP.
*/

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

//UDP handle
WiFiUDP Udp;
unsigned int UDP_Port = 5000;
char *UDP_IP = "192.168.4.1"; //IP address of the RECEIVER
char c;
char PacketBuffer1[32]; //Buffer used for UDP data
char PacketBuffer2[32]; //Buffer used for UDP data

//Wifi handle
const char *ssid = "SSID";
const char *password = "PASSWORD";
IPAddress ip(192, 168, 4, 2); //set a static IP for this device
IPAddress gateway(192, 168, 4, 1);
IPAddress subnet(255, 255, 255, 0);

bool flag = false; //false = cingo||||true = arm
bool first = true;
int i = 0;

void setup()
{
	memset(&PacketBuffer1, (char)0, 32); //set all buffer to 0
	memset(&PacketBuffer2, (char)0, 32); //set all buffer to 0

	pinMode(LED_BUILTIN, OUTPUT); //LOW = WiFi connected; HIGH = WiFi not connected
	digitalWrite(LED_BUILTIN, HIGH);
	Serial.begin(115200); //BLE serial
	WiFi.config(ip, gateway, subnet);
	WiFi.mode(WIFI_STA); //station mode
	WiFi.begin(ssid, password);
	Serial.println();
	Serial.print("Wait for WiFi");
	//wait for wireless connection
	while (WiFi.status() != WL_CONNECTED)
	{
		delay(500);
		Serial.print(".");
	}
	digitalWrite(LED_BUILTIN, LOW);
	Serial.println("");
	Serial.println("WiFi connected");
	Serial.println("IP address: " + WiFi.localIP().toString());
	Serial.println();

	Udp.begin(UDP_Port); //initialize UDP
}

void loop()
{
	//if wireless connection drops, try to reconnect to it
	while (WiFi.status() != WL_CONNECTED)
	{
		digitalWrite(LED_BUILTIN, HIGH);
		delay(500);
		Serial.print(".");
	}

	digitalWrite(LED_BUILTIN, LOW);

	if (Serial.available())
	{
		c = Serial.read();
		//Serial.print(c); //debug
		//handle the remaining data for packet and when the '\n' char is found the enite char array can be sent via UDP
		if ((first == false) && (flag == false))
		{
			PacketBuffer1[i] = c;
			if (c == '\n')
			{
				i = 0;
				first = true;
				Serial.print(PacketBuffer1);
				Udp.beginPacket(UDP_IP, UDP_Port);
				Udp.write(PacketBuffer1);
				Udp.endPacket();
				memset(PacketBuffer1, (char)0, 32);
			}
			else
				i++;
		}
		else if ((first == false) && (flag == true))
		{
			PacketBuffer2[i] = c;
			if (c == '\n')
			{
				i = 0;
				first = true;
				Serial.print(PacketBuffer2);
				Udp.beginPacket(UDP_IP, UDP_Port);
				Udp.write(PacketBuffer2);
				Udp.endPacket();
				memset(PacketBuffer2, (char)0, 32);
			}
			else
				i++;
		}

		//handle the first char of a complete packet and checks into which of the buffers data have to be stored
		//CINGO
		if ((c == 'A') && (first == true))
		{
			PacketBuffer1[i] = c;
			i++;
			flag = false;
			first = false;
		}
		//ARM
		else if ((c == 'B') && (first == true))
		{
			PacketBuffer2[i] = c;
			i++;
			flag = true;
			first = false;
		}
	}
}