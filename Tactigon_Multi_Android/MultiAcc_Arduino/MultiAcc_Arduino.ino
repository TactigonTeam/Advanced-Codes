#include <tactigon_led.h>
#include <tactigon_IMU.h>
#include <tactigon_BLE.h>
#include <tactigon_Battery.h>
#include <tactigon_Env.h>
#include <tactigon_UserSerial.h>

T_Led rLed, bLed, gLed;

T_ACC accMeter;
T_AccData accData;

T_GYRO gyroMeter;
T_GyroData gyroData;

T_MAG magMeter;
T_MagData magData;

T_QUAT qMeter;
T_QData qData;

T_Battery bMeter;
T_BattData bData;

T_EnvSens eMeter;
T_EnvData eData;

T_BLE bleManager;
UUID uuid;
T_BLE_Characteristic accChar, gyroChar, envChar, qChar, cmdChar;

T_UserSerial tSerial;

int ticks, ticksLed, stp;

/*----------------------------------------------------------------------*/
void setup()
{

  char charProg;

  // put your setup code here, to run once:
  ticks = 0;
  ticksLed = 0;
  stp = 0;

  rLed.init(T_Led::RED);
  gLed.init(T_Led::GREEN);
  bLed.init(T_Led::BLUE);

  rLed.off();
  gLed.off();
  bLed.off();

  //init name
  bleManager.setName("DEMO2");

  //init role
  bleManager.InitRole(TACTIGON_BLE_PERIPHERAL);

  //add characteristic
  uuid.set("bea5760d-503d-4920-b000-101e7306b007"); //acc
  accChar = bleManager.addNewChar(uuid, 14);

  uuid.set("bea5760d-503d-4920-b000-101e7306b008"); //gyro
  gyroChar = bleManager.addNewChar(uuid, 14);

  uuid.set("bea5760d-503d-4920-b000-101e7306b009"); //environment
  envChar = bleManager.addNewChar(uuid, 10);

  uuid.set("bea5760d-503d-4920-b000-101e7306b005"); //quaternions as float (used also by Cube app)
  qChar = bleManager.addNewChar(uuid, 18);

  //disable use of magnetometer in quat computing
  qMeter.enable();
  qMeter.useMag(0);
}

/*----------------------------------------------------------------------*/
void loop()
{

  unsigned char buffData[24];
  char stringBuffer[64];
  float roll, pitch, yaw;

  //update BLE characteristics @ 50Hz
  if (millis() >= (ticks + (1000 / 50)))
  {
    ticks = millis();

    //get data
    accData = accMeter.getAxis();
    gyroData = gyroMeter.getAxis();
    magData = magMeter.getAxis();
    qData = qMeter.getQs();
    eData = eMeter.getData();

    //update acc char
    memset(buffData, 0, sizeof(buffData));
    memcpy(&buffData[2], &accData.x, 4);
    memcpy(&buffData[6], &accData.y, 4);
    memcpy(&buffData[10], &accData.z, 4);
    accChar.update(buffData);

    //update gyro char
    memset(buffData, 0, sizeof(buffData));
    memcpy(&buffData[2], &gyroData.x, 4);
    memcpy(&buffData[6], &gyroData.y, 4);
    memcpy(&buffData[10], &gyroData.z, 4);
    gyroChar.update(buffData);

    //update env char
    memset(buffData, 0, sizeof(buffData));
    memcpy(&buffData[2], &eData.temp, 4);
    memcpy(&buffData[6], &eData.press, 4);
    envChar.update(buffData);

    //update quat char with angles
    //convert Euler rad -> deg
    roll = rad2Deg(qData.roll);
    pitch = rad2Deg(qData.pitch);
    yaw = rad2Deg(qData.yaw);

    //remap roll
    roll = rollZeroRemap(roll);

    //sprintf(stringBuffer, "%d   %d   %d", (int)roll, (int)pitch, (int)yaw);
    //Serial.println(stringBuffer);

    memset(buffData, 0, sizeof(buffData));
    memcpy(&buffData[2], &roll, 4);
    memcpy(&buffData[6], &pitch, 4);
    memcpy(&buffData[10], &yaw, 4);
    qChar.update(buffData);
  }

  //rotate rgb led and print accelerometer FullScale
  if (millis() >= (ticksLed + (1000 / 1)))
  {
    unsigned char buff[32];
    int i;
    float fs;
    int decVal;

    ticksLed = millis();

    //led handling
    if (stp == 0)
    {
      rLed.on();
      gLed.off();
      bLed.off();
    }
    else if (stp == 1)
    {
      rLed.off();
      gLed.on();
      bLed.off();
    }
    else if (stp == 2)
    {
      rLed.off();
      gLed.off();
      bLed.on();
    }

    stp = (stp + 1) % 3;
  }
}

/*----------------------------------------------------------------------
 *
 * remap roll in order to have 0 deg in horizontal position
 * and +180,-180 periodicity around this zero position
 *
 *----------------------------------------------------------------------*/
float rollZeroRemap(float r)
{
  if ((r > -180) && (r < 90))
    r = r + 90;
  else
    r = -270 + r;

  return r;
}

/*-----------------------------------------------------------------*/
float rad2Deg(float rad)
{
  return rad * 180 / PI;
}