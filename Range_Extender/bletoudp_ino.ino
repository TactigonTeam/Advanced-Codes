//SENDER
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

//UDP handle
WiFiUDP Udp;
unsigned int UDP_Port = 5000;
char *UDP_IP = "192.168.4.1"; //IP address of the RECEIVER
char PacketBuffer[1024];	  //Buffer used for UDP data

//Wifi handle
const char *ssid = "YOUR_SSID";
const char *password = "YOUR_PASSWORD";
IPAddress ip(192, 168, 4, 2); //set a static IP for this device
IPAddress gateway(192, 168, 4, 1);
IPAddress subnet(255, 255, 255, 0);

bool flag = false; //to handle complete read of BLE data
int count = 0;	   //counter for checking correct length of received buffer from BLE, starts from 0 to 19

void setup()
{
	memset(&PacketBuffer, (char)0, 1024); //set all buffer to 0
	pinMode(LED_BUILTIN, OUTPUT);		  //LOW = WiFi connected; HIGH = WiFi not connected
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
		//read one char at the time and store it to che progressive 'count' position in the buffer array
		Serial.read(&PacketBuffer[count], 1);
		//checking for carriage return and line feed chars
		//replace carriage return (if for any reasons is present) with whitespace to avoid complications in the buffer processing by the receiver
		if (PacketBuffer[count] == '\r')
			PacketBuffer[count] = ' ';
		else if (PacketBuffer[count] == '\n')
		{
			//at this point the one buffer from BLE serial is completely processed
			PacketBuffer[count] = (char)0;
			count = 0;	 //reset counter
			flag = true; //complete data
		}
		else
		{
			//increment counter for next char read
			count++;
		}
	}

	if (flag)
	{
		//start send data from [1] and not [0] due to how data is sent by the T-skin.
		//the data in [0] is treated by a serial read as a terminator char (char)0.
		//if this data ends up in the buffer that we send the calid data after that char will be ignored
		//sending data from 2nd element is less time consuming that shifting all buffer
		//Serial.println(&PacketBuffer[1]); //for debug
		//here we send via UDP the data from BLE
		Udp.beginPacket(UDP_IP, UDP_Port);
		Udp.write(&PacketBuffer[1]);
		Udp.endPacket();
		flag = false;						 //reset flag for next buffer
		memset(PacketBuffer, (char)0, 1024); //set all buffer to 0
	}
}