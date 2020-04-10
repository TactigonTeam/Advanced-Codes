/*
  Name:    RoboticArm_TSkin.ino
  Author:  Luca Seva'
  Description: This code is responsable of controlling the robotic arm using a T-Skin
*/

#include <tactigon_led.h>
#include <tactigon_IMU.h>
#include <tactigon_BLE.h>
#include <tactigon_IO.h>

#define ROLL_WINDOW 20
#define PITCH_WINDOW 20
#define YAW_NOT_A_ZERO_VAL 1000000 //every value not included between -180 and 180

#define ROLL_SENSITIVITY 0.5

//Check the right MAC Address and UART Characteristic of your BLE to UART module. Check the Baudrate on the Arduino-braccio.ino sketch too.
#define TARGET_MAC                     \
  {                                    \
    0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF \
  }
#define TARGET_CHAR "target characteristic"
//LEDs
T_Led rLed, bLed, gLed;
//Accelerometer
T_ACC accMeter;
T_AccData accData;
//Gyroscope
T_GYRO gyro;
//Quaterions and euler's angles
T_QUAT qMeter;
T_QData qData;
//BLE
T_BLE bleManager;
//GPPs for buttons
T_GPP gpp1;
T_GPP gpp2;
T_GPP gpp3;
T_GPP gpp4;

//Target mac address
uint8_t targetMAC[6] = TARGET_MAC;

UUID targetUUID;

//Transmission flag
boolean gTransmission = false;
//Nipple grip = 0
//Wrist rotation = 1
int gNippleWrist = 0;
//Connection status
int gConnStatus = 0;

//angles computed for the message to be sent
int16_t a1_shoulder, a2_elbow, a3_wrist, a4_nipper, a5_base, crc;
//Temporary zero for yaw (increases dramatically precision)
float gYawZero;
//Base rotation enabling
int gBaseRotEnable = 0;

/////////////////////////////////////////////////////////////////////////////////
void setup()
{
  //init leds
  rLed.init(T_Led::RED);
  gLed.init(T_Led::GREEN);
  bLed.init(T_Led::BLUE);
  rLed.off();
  gLed.off();
  bLed.off();

  //init CENTRAL mode
  bleManager.InitRole(TACTIGON_BLE_CENTRAL);
  targetUUID.set(TARGET_CHAR);
  bleManager.setTarget(targetMAC, targetUUID);

  //init GPIO as inputs
  gpp1.init(T_GPP::GPP1, T_GPP::GPP_IN);
  gpp2.init(T_GPP::GPP2, T_GPP::GPP_IN);
  gpp3.init(T_GPP::GPP3, T_GPP::GPP_IN);
  gpp4.init(T_GPP::GPP4, T_GPP::GPP_IN);

  //init sensor fusion
  qMeter.useMag(0);
  qMeter.enable();

  //init arm angles
  a2_elbow = a3_wrist = a4_nipper = crc = 0;
  a5_base = 90;
  a1_shoulder = -90;
}

/////////////////////////////////////////////////////////////////////////////////
void loop()
{
  static uint32_t ticksProc = 0;

  //wait for IMU startup
  if (initIMU() == 0)
  {
    return;
  }

  //main loop computed every 300ms
  if (millis() >= (ticksProc + 300))
  {
    char bleBuff[32];

    ticksProc = millis();

    //get quaternions data
    qData = qMeter.getQs();

    //shoulder
    if (abs(radToDeg(qData.pitch)) < 60)
      shoulderHandling(qData.roll);

    //base
    baseHandling(qData.yaw);

    if (gNippleWrist == 0)
    {
      //gripper (open/close)
      nippleHandling(qData.pitch * 1.5);
    }
    else
    {
      //wrist rotation
      wristHandling(qData.pitch * 1.5);
    }

    //prepare buffer for ble
    sprintf(bleBuff, "B%d,%d,%d,%d,%d %d\n", a1_shoulder, a2_elbow, a3_wrist, a4_nipper, a5_base, crc);
    Serial.println(bleBuff);

    //transmit
    gConnStatus = bleManager.getStatus();
    if (gConnStatus == 3)
    {
      if (gTransmission)
        bleManager.writeToPeripheral((unsigned char *)bleBuff, strlen(bleBuff));
    }
  }

  //led handling
  ledHandling(gConnStatus);

  //debounced buttons handling
  buttonsHandling();
}

////////////////////////////////////////////////////////////////
void shoulderHandling(float roll)
{
  float r;
  static int gRollPos;

  //remap zero
  r = rollZeroRemap(radToDeg(roll)); //zero in horizontal position with period +/- 180

  //if roll is greater than ROLL_WINDOW
  if (fabs(r) > ROLL_WINDOW)
  {
    //subtract window value
    r = (fabs(r) - ROLL_WINDOW) * my_f_sign(r);
    gRollPos = gRollPos + r * ROLL_SENSITIVITY;

    //limits
    if (gRollPos > 20) //maximum retraction
      gRollPos = 20;
    else if (gRollPos < -50) //maximum distension
      gRollPos = -50;

    //update global
    a1_shoulder = -gRollPos - 90;
  }
}

////////////////////////////////////////////////////////////////
void nippleHandling(float pitch)
{
  float p;
  static int gPitchPos = 0;

  //remap zero: nothing to do as pitch is already zero in horizontal position
  p = radToDeg(pitch);

  //if pitch is greater than PITCH_WINDOW
  if (fabs(p) > PITCH_WINDOW)
  {
    //subtract window value
    p = (fabs(p) - PITCH_WINDOW) * my_f_sign(p);
    gPitchPos = gPitchPos + p;

    //limits
    if (gPitchPos > 60) //gripper open
      gPitchPos = 60;
    else if (gPitchPos < -20) //gripper closed
      gPitchPos = -20;

    //update global
    a4_nipper = gPitchPos;
  }
}

////////////////////////////////////////////////////////////////
void wristHandling(float pitch)
{
  float p;
  static int gWristPos = 0;

  //remap zero: nothing to do as pitch is already zero in horizontal position
  p = radToDeg(pitch);

  //if pitch is greater than PITCH_WINDOW
  if (fabs(p) > PITCH_WINDOW)
  {
    //subtract window value
    p = (fabs(p) - PITCH_WINDOW) * my_f_sign(p);
    gWristPos = gWristPos + p;

    //limits
    if (gWristPos > 100) //gripper open
      gWristPos = 90;
    else if (gWristPos < -90) //gripper closed
      gWristPos = -100;

    //update global
    a3_wrist = gWristPos;
  }
}

////////////////////////////////////////////////////////////////
void baseHandling(float yaw)
{
  float y;
  //if rotation is enabled
  if (gBaseRotEnable)
  {
    //compute the degree covered while rotation is enabled
    y = radToDeg(yaw);
    y = yawZeroRemap(y, gYawZero);
    //update global
    a5_base = -y + 90; // -yaw + 90
  }
}

////////////////////////////////////////////////////////////////
void buttonsHandling()
{
  static uint32_t debounceGpp4 = 0;
  static uint32_t debounceGpp1 = 0;
  static uint32_t debounceGpp2 = 0;
  static int debounceTime = 300;

  //debounce on GPP4 (activate/deactivate transmission) - sticky button
  if (!gpp4.read())
  {
    if (millis() >= debounceGpp4 + debounceTime)
    {
      //toggle transmission status
      gTransmission = !gTransmission;
      debounceGpp4 = millis();
    }
  }

  //debounce on GPP2 (switch nipple/wrist) - sticky button
  if (!gpp2.read())
  {
    if (millis() >= debounceGpp2 + debounceTime)
    {
      //toggle nipple/wrist control
      gNippleWrist = !gNippleWrist;
      debounceGpp2 = millis();
    }
  }

  //debounce on GPP1 (activate /deactivate base rotation)
  if (millis() >= debounceGpp1 + debounceTime)
  {
    int pressed;

    debounceGpp1 = millis();

    pressed = !gpp1.read();
    if (pressed)
    {
      if (gYawZero == YAW_NOT_A_ZERO_VAL) //assumes yaw = 0 in the instant it has been pressed
      {
        gYawZero = radToDeg(qData.yaw);
      }
    }
    else
      gYawZero = YAW_NOT_A_ZERO_VAL;

    //update global
    gBaseRotEnable = pressed;
  }
}

////////////////////////////////////////////////////////////////
float radToDeg(float rad)
{
  //rad to deg conversion
  return rad * 360 / 6.28;
}

//////////////////////////////////////////////////////////////////////////
//
// roll remap to have 0 in horizontal position
// -180, +180 period around this 0
//
//////////////////////////////////////////////////////////////////////////
float rollZeroRemap(float r)
{
  if ((r > -180) && (r < 90))
    r = r + 90;
  else
    r = -270 + r;

  return r;
}

//////////////////////////////////////////////////////////////////////////
//
// yaw remap to have yaw = 0 in the position passed
// -180, +180 period around this 0
//
//////////////////////////////////////////////////////////////////////////
float yawZeroRemap(float y, float zero)
{
  if (zero <= 0)
  {
    if ((y > -180) && (y <= (180 + zero)))
      y = y - zero;
    else
      y = -360 + y - zero;
  }
  else
  {
    if ((y > (-180 + zero)) && (y <= 180))
      y = y - zero;
    else
      y = 360 + y - zero;
  }

  return y;
}

//////////////////////////////////////////////////////////////////////////
//
// returns float sign
//
//////////////////////////////////////////////////////////////////////////
float my_f_sign(float x)
{
  return ((x > 0) - (x < 0));
}

//////////////////////////////////////////////////////////////////////////
void ledHandling(int connStatus)
{
  static uint32_t ticksLed = 0;
  static int stp = 0;

  if (millis() >= (ticksLed + 500))
  {
    ticksLed = millis();

    switch (connStatus)
    {
    case 0:
      if (stp == 0)
      {
        rLed.on();
        gLed.off();
        bLed.off();
      }
      else if (stp == 1)
      {
        rLed.off();
        gLed.off();
        bLed.on();
      }
      else if (stp == 2)
      {
        rLed.off();
        gLed.on();
        bLed.off();
      }
      stp = (stp + 1) % 3;
      break;

    case 1: //seraching for target
      rLed.on();
      gLed.off();
      bLed.off();
      break;

    case 2: //target found, searching for characteristic
      rLed.off();
      gLed.off();
      bLed.on();
      break;

    case 3: //ready for characteristic operations
      rLed.off();
      gLed.on();
      if (gTransmission)
        bLed.on();
      else
        bLed.off();
      break;
    }
  }
}

int initIMU(void)
{
  static uint32_t ticksInit = 0;
  static int done = 0;
  static int cnt = 0;
  static int ledState = 1;
  static int stepCnt = 0;
  float r;

  if (done == 0)
  {
    if (millis() >= (ticksInit + 100))
    {
      ticksInit = millis();

      //wait 1 second to let sensor fusion begin
      if (stepCnt < 10)
      {
        stepCnt++;
        return done;
      }

      //
      qData = qMeter.getQs();
      r = rollZeroRemap(radToDeg(qData.roll));
      if (fabs(r) > ROLL_WINDOW)
      {
        //led state machine: 300msec on, 500msec off
        if (ledState == 1)
        {
          rLed.on();
          if (cnt++ == 2)
          {
            ledState = 0;
            cnt = 0;
          }
        }
        else
        {
          rLed.off();
          if (cnt++ == 4)
          {
            ledState = 1;
            cnt = 0;
          }
        }
      }
      else
        done = 1;
    }
  }

  return done;
}