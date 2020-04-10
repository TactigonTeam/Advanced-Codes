#include <tactigon_led.h>
#include <tactigon_IMU.h>
#include <tactigon_BLE.h>
#include <tactigon_IO.h>

typedef struct
{
  int stat;
  int lastEventMillis;
  int debounceTime;
  T_GPP io;
} DebouncedButton;

T_Led rLed, bLed, gLed;
T_GYRO gyroMeter;

T_QUAT qMeter;
T_QData qData;

T_BLE bleManager;
UUID uuid;
T_BLE_Characteristic bleChar;
T_GPP tGPP_1, tGPP_2, tGPP_3, tGPP_4;

int ticks, cnt;

//init debounced buttons structures (actually unused in this application)
DebouncedButton b1 = {0, 0, 300, tGPP_1};
DebouncedButton b2 = {0, 0, 300, tGPP_2};
DebouncedButton b3 = {0, 0, 300, tGPP_3};
DebouncedButton b4 = {0, 0, 300, tGPP_4};

////////////////////////////////////////////////////////////////////////////////////////
void setup()
{

  ticks = 0;
  rLed.init(T_Led::RED);
  gLed.init(T_Led::GREEN);
  bLed.init(T_Led::BLUE);

  rLed.off();
  gLed.off();
  bLed.off();

  //set ble name
  bleManager.setName("Test1");

  //init ble role
  bleManager.InitRole(TACTIGON_BLE_PERIPHERAL);

  //add acc characteristic
  uuid.set("bea5760d-503d-4920-b000-101e7306b005");
  bleChar = bleManager.addNewChar(uuid, 17);

  //disable use of magnetometer in quaternions computing
  qMeter.enable();
  qMeter.useMag(0);

  //init GPIO class
  b1.io.init(T_GPP::GPP1, T_GPP::GPP_IN);
  b2.io.init(T_GPP::GPP2, T_GPP::GPP_IN);
  b3.io.init(T_GPP::GPP3, T_GPP::GPP_IN);
  b4.io.init(T_GPP::GPP4, T_GPP::GPP_IN);

  //gyro calib
  //gyroMeter.startCalib();
}

////////////////////////////////////////////////////////////////////////////////////////
void loop()
{
  unsigned char buffData[24];
  unsigned char btnWord;

  //evaluate buttons
  evalDeboucedButton(&b1);
  evalDeboucedButton(&b2);
  evalDeboucedButton(&b3);
  evalDeboucedButton(&b4);

  //build buttons word
  btnWord = 0;
  btnWord += b1.stat * 8;
  btnWord += b2.stat * 4;
  btnWord += b3.stat * 2;
  btnWord += b4.stat * 1;

  //update BLE characteristic @ 50Hz
  if (GetCurrentMilli() >= (ticks + (1000 / 50)))
  {
    ticks = GetCurrentMilli();

    //get quaternions data
    qData = qMeter.getQs();

    //prepare buffer
    memset(buffData, 0, sizeof(buffData));

    memcpy(&buffData[0], &qData.q0f, 4);  //quaternions
    memcpy(&buffData[4], &qData.q1f, 4);  //
    memcpy(&buffData[8], &qData.q2f, 4);  //
    memcpy(&buffData[12], &qData.q3f, 4); //

    memcpy(&buffData[16], &btnWord, 1); //

    //update ble characteristic
    bleChar.update(buffData);
  }

  //led handling
  ledHandling();
}

////////////////////////////////////////////////////////////////////////////////////////
void ledHandling()
{
  static int ticksLed = 0;

  if (millis() >= (ticksLed + 500))
  {
    int bleStat;

    ticksLed = millis();

    //get ble status
    bleStat = bleManager.getStatus();

    //set led depending on connection status
    if (bleStat == 0)
    {
      //not connected: red led
      rLed.on();
      gLed.off();
      bLed.off();
    }
    else
    {
      //connected: blue led
      rLed.off();
      gLed.off();
      bLed.on();
    }
  }
}

////////////////////////////////////////////////////////////////////////////////////////
void evalDeboucedButton(DebouncedButton *pButton)
{
  DebouncedButton b;
  int curVal;

  b = *pButton;

  curVal = b.io.read();
  if (curVal != b.stat)
  {
    if (millis() >= b.lastEventMillis + b.debounceTime)
    {
      b.stat = curVal;
      b.lastEventMillis = millis();
    }
  }

  *pButton = b;
}
