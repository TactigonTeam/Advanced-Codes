/*
  Name:    T-Skin_Rover.ino
  Author:  Luca Seva'
  Description: This code is responsable for the control of the tracked rover using a T-Skin
*/

#include <tactigon_led.h>
#include <tactigon_IMU.h>
#include <tactigon_BLE.h>
#include <tactigon_IO.h>

extern int ButtonPressed;

T_Led rLed, bLed, gLed;

T_QUAT qMeter;
T_QData qData;

T_BLE bleManager;
UUID targetUUID;
uint8_t targetMAC[6] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};
T_BLE_Characteristic accChar, gyroChar, magChar, qChar;

//GPIO
T_GPP gpp1;
T_GPP gpp2;
T_GPP gpp3;
T_GPP gpp4;

boolean transmission = true; //change to true to use tactigon one
int debounceTime = 300;
int lastTransmissionSwitch, lastGPIO1Switch, lastGPIO2Switch, lastGPIO3Switch = 0;

int ticks, ticksLed, stp, cnt, printCnt, ledCnt;
float roll, pitch, yaw;

//////////////////////////////////////////////////////////////////////////////////////////
void setup()
{
  // put your setup code here, to run once:
  ticks = 0;
  ticksLed = 0;
  stp = 0;
  cnt = 0;
  ledCnt = 0;

  //init leds
  rLed.init(T_Led::RED);
  gLed.init(T_Led::GREEN);
  bLed.init(T_Led::BLUE);
  rLed.off();
  gLed.off();
  bLed.off();

  //init BLE
  bleManager.setName("Tacti");
  bleManager.InitRole(TACTIGON_BLE_CENTRAL);   //BLE role: CENTRAL
  targetUUID.set("target characteristic");     //target characteristic
  bleManager.setTarget(targetMAC, targetUUID); //target: mac device and its char UUID

  gpp1.init(T_GPP::GPP1, T_GPP::GPP_IN);
  gpp2.init(T_GPP::GPP2, T_GPP::GPP_IN);
  gpp3.init(T_GPP::GPP3, T_GPP::GPP_IN);
  gpp4.init(T_GPP::GPP4, T_GPP::GPP_IN);
}

//////////////////////////////////////////////////////////////////////////////////////////
void loop()
{
  char buffData[4];
  buffData[0] = 'A';
  buffData[3] = '\n';

  int deltaWheel, speedWheel;
  int pitchThreshold, rollThreshold, th1, th2;

  //base engine @ 50Hz (20msec)
  if (millis() >= (ticks + (1000 / 50)))
  {
    ticks = millis();
    //debounce on GPP4 (activate /deactivate transmission)
    if (!gpp4.read())
    {
      if (millis() >= lastTransmissionSwitch + debounceTime)
      {
        transmission = !transmission;
        lastTransmissionSwitch = millis();
      }
    }
    if (transmission)
    {
      //get quaternions and Euler angles
      qData = qMeter.getQs();

      //Euler angles: rad/sec --> degrees/sec
      roll = qData.roll * 360 / 6.28;
      pitch = qData.pitch * 360 / 6.28;
      yaw = qData.yaw * 360 / 6.28;

      //forward/backword
      rollThreshold = 10;
      th1 = 90 + rollThreshold;
      th2 = 90 - rollThreshold;
      roll = fabs(roll);

      //compute speed wheel and delta speed wheel depending on Euler angles
      //left/right
      pitchThreshold = 10;
      if (pitch < -pitchThreshold || pitch > pitchThreshold)
      {
        if (pitch < -pitchThreshold)
        {
          if (roll < th1 && roll > th2)
          {
            //spin
            deltaWheel = -(fabs(pitch) - pitchThreshold) * 10;
            rLed.on();
            bLed.on();
            gLed.on();
          }
          else
          {
            deltaWheel = -(fabs(pitch) - pitchThreshold) * 3;
          }
        }
        else
        {
          if (roll < th1 && roll > th2)
          {
            //spin
            deltaWheel = +(fabs(pitch) - pitchThreshold) * 10;
            rLed.on();
            bLed.on();
            gLed.on();
          }
          else
          {
            deltaWheel = +(fabs(pitch) - pitchThreshold) * 3;
          }
        }
      }
      else
      {
        deltaWheel = 0;
      }

      //forward/backward
      if (roll > th1)
      {
        speedWheel = ((roll - th1) * 3);
      }
      else if (roll < th2)
      {
        speedWheel = ((roll - th2) * 3);
      }
      else
      {
        speedWheel = 0;
      }

      //convert speedWheel and deltaWheel in command byte for motor control board
      int sxC, dxC;
      uint8_t sx, dx;
      sxC = (speedWheel - (-deltaWheel / 4)) + 64;
      dxC = (speedWheel + (-deltaWheel / 4)) + 192;

      if (sxC > 127)
      {
        sxC = 127;
      }
      else if (sxC < 1)
      {
        sxC = 1;
      }
      if (dxC < 128)
      {
        dxC = 128;
      }
      else if (dxC > 255)
      {
        dxC = 255;
      }
      sx = sxC;
      dx = dxC;

      //fill buffData to write in BLE characteristic
      buffData[1] = sx;
      buffData[2] = dx;
      Serial.print("SX: ");
      Serial.println(sx);
      Serial.print("DX: ");
      Serial.println(dx);
      //if connected and attached to peripheral characteristic write in it
      if (bleManager.getStatus() == 3)
      {

        //signal that connection is on
        bLed.on();
        rLed.off();
        ledCnt++;
        if (ledCnt > 100)
        {
          ledCnt = 0;
          rLed.off();
          bLed.off();
          gLed.off();
        }
        //write in BLE characteristic (@ 10Hz in order to not stress control motor board)
        cnt++;
        if (cnt > 5)
        {
          bleManager.writeToPeripheral((unsigned char *)buffData, 4);
          cnt = 0;
          rLed.on();
        }
      }
    }
    else
    {
      buffData[1] = 0x00;
      buffData[2] = 0x00;
      //debounce on GPP1 (rotate)
      if (!gpp2.read())
      {
        if (millis() >= lastTransmissionSwitch + debounceTime)
        {
          buffData[1] = 0x01;
          buffData[2] = 0xFF;
          lastGPIO1Switch = millis();
        }
      }
      //debounce on GPP2 (rotate)
      if (!gpp1.read())
      {
        if (millis() >= lastTransmissionSwitch + debounceTime)
        {
          buffData[1] = 0x7F;
          buffData[2] = 0x80;
          lastGPIO2Switch = millis();
        }
      }
      //debounce on GPP3 (stop)
      if (!gpp3.read())
      {
        if (millis() >= lastTransmissionSwitch + debounceTime)
        {
          buffData[1] = 0x00;
          buffData[2] = 0x00;
          lastGPIO3Switch = millis();
        }
      }
      bleManager.writeToPeripheral((unsigned char *)buffData, 2);
    }
  }
}