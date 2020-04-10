/*
 Name:		Nantucket-Mega.ino
 Author:	Luca Seva'
 Description:	An Arduino Mega, thanks to its 3 additional hardware serial ports function as a safe harbour for data from 2 BLE modules.
				When a data buffer is complete it's shipped to another serial to a nodemcu that will transmit the data via the UDP protocol.
*/

char CINGOData[4];
char ARMData[32];
int i = 0, j = 0;

void setup()
{
	memset(CINGOData, char(0), sizeof(CINGOData));
	memset(ARMData, char(0), sizeof(ARMData));
	//Serial.begin(115200); //debug serial port
	Serial1.begin(115200); //NodeMCU
	Serial2.begin(115200); //BLE #1 [CINGO]
	Serial3.begin(115200); //BLE #2 [ARM]
}

void loop()
{
	if (Serial2.available())
	{
		CINGOData[i] = Serial2.read();
		if (CINGOData[i] == '\n')
		{
			i = 0;
			//the buffer is full so we can relay it to the nodemcu that will send it over upd
			//Serial.print(CINGOData); //debug
			Serial1.print(CINGOData);
			memset(CINGOData, char(0), sizeof(CINGOData));
		}
		else
			i++;
	}

	if (Serial3.available())
	{
		ARMData[j] = Serial3.read();
		if (ARMData[j] == '\n')
		{
			j = 0;
			//the buffer is full so we can relay it to the nodemcu that will send it over upd
			//Serial.print(ARMData); //debug
			Serial1.print(ARMData);
			memset(ARMData, char(0), sizeof(ARMData));
		}
		else
			j++;
	}
}