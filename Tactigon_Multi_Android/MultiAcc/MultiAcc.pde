import blepdroid.*;
import blepdroid.BlepdroidDevice;
import com.lannbox.rfduinotest.*;
import android.os.Bundle;
import android.content.Context;
import java.util.UUID;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;



Blepdroid blepdroid;

boolean targetCharacteristicFound = false;

String[] DEVICES = {"DEMO1", "DEMO2", "DEMO3"};
String[] DEVICES_CHAR_TO_READ = {"bea5760d-503d-4920-b000-101e7306b007", "bea5760d-503d-4920-b000-101e7306b009", "bea5760d-503d-4920-b000-101e7306b005"};



int scanMillis = 0;




///////////////////////////////////////////////////////////////////////////////////////////
void setup() {
  //size(600, 800);
  fullScreen();
  smooth();
  println(" OK ");

  //crea classe cle
  blepdroid = new Blepdroid(this);
  
  demo1_init();
  demo2_init();
  demo3_init();
}


///////////////////////////////////////////////////////////////////////////////////////////
void draw() 
{

  background(250, 250, 250);
    
    
  //plot
  demo1_plot();
  demo2_plot();
  demo3_plot();
    
    

  
  //
  scanForDevices();
}





//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void scanForDevices()
{  
  int currentTime = millis();
  
  if(millis() >= (scanMillis + 2000))
  {    
    scanMillis = millis();
    
    println(" scan !");
    blepdroid.scanDevices();
  }
  
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void onDeviceDiscovered(BlepdroidDevice device)
{
  println("discovered device " + device.name + " address: " + device.address + " rssi: " + device.rssi );

  for(int i=0; i<DEVICES.length; i++)
  {
    String dev = DEVICES[i];
    
    if (device.name != null && device.name.equals(dev))
    {
      println(dev + " Found");
      
      if (blepdroid.connectDevice(device))
      {
        println(" connected to " + dev);      
      } else
      {
        println(" couldn't connect device " + dev);
      }
    } 
  }
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void onServicesDiscovered(BlepdroidDevice device, int status)
{
  
  HashMap<String, ArrayList<String>> servicesAndCharas = blepdroid.findAllServicesCharacteristics(device);
  
  for( String service : servicesAndCharas.keySet())
  {
    print( service + " has " );
    
    // this will list the UUIDs of each service, in the future we're going to make
    // this tell you more about each characteristic, e.g. whether it's readable or writable
    //println( servicesAndCharas.get(service));
    
    int i;
    for(i=0; i<DEVICES.length; i++)
    {
      if(device.name.equals(DEVICES[i]))
        break;
    }
    
    if(i < DEVICES.length)
    {
      String charString = DEVICES_CHAR_TO_READ[i];
      
      for(String charact: servicesAndCharas.get(service))
      {
        print( " charact: " + charact);
        if(charact.equals(charString))
        {
          targetCharacteristicFound = true;
          print( " target characteristic found! ");
          blepdroid.setCharacteristicToListen(device, UUID.fromString(charString));
        }
      }
    }
    
  }
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// these are all the BLE callbacks
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void onBluetoothRSSI(BlepdroidDevice device, int rssi)
{
  println(" onBluetoothRSSI " + device.address + " " + Integer.toString(rssi));
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void onBluetoothConnection( BlepdroidDevice device, int state)
{
  blepdroid.discoverServices(device);
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void onCharacteristicChanged(BlepdroidDevice device, String characteristic, byte[] data)
{  
  if(device.name.equals(DEVICES[0]))
  {
    demo1_CB(data);    
  }
   
  if(device.name.equals(DEVICES[1]))
  {
    demo2_CB(data);
  }
  
  if(device.name.equals(DEVICES[2]))
  {
    demo3_CB(data);
  }
  
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void onDescriptorWrite(BlepdroidDevice device, String characteristic, String data)
{
  println(" onDescriptorWrite " + characteristic + " " + data);
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void onDescriptorRead(BlepdroidDevice device, String characteristic, String data)
{
  println(" onDescriptorRead " + characteristic + " " + data);
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void onCharacteristicRead(BlepdroidDevice device, String characteristic, byte[] data)
{
  println(" onCharacteristicRead " + characteristic + " " + data);
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void onCharacteristicWrite(BlepdroidDevice device, String characteristic, byte[] data)
{
  println(" onCharacteristicWrite " + characteristic + " " + data);
}
