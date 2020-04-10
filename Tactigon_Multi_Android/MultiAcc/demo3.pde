
short[] rArray;
short[] pArray;

float rollCurr = 0;
float pitchCurr = 0;

int myWidth3;
int myEight3;
int vertOffset3;
int border3 = 10;



//////////////////////////////////////////////////////////////////////////////////////
void demo3_init()
{
  myWidth3 = width;
  myEight3 = height / 3 - 2 * border3;
  vertOffset3 = 2 * height / 3;
  
  //inizializza array con i punti da plottare
  rArray = new short[myWidth3];  
  pArray = new short[myWidth3];
  for(int i=0; i<myWidth3; i++)
  {
    rArray[i] = (short)((myEight3/2) + vertOffset3);
    pArray[i] = (short)((myEight3/2) + vertOffset3);
  }
}



//////////////////////////////////////////////////////////////////////////////////////
void demo3_CB(byte[] data)
{
    //  | timeStamp(2) | roll(4) | pitch(4) | yaw(4) |
  
    // get each set of two bytes
    byte[] rr = new byte[4];
    byte[] pp = new byte[4];
    byte[] yy = new byte[4];
  
    System.arraycopy( data, 2, rr, 0, 4 );
    System.arraycopy( data, 6, pp, 0, 4 );
    System.arraycopy( data, 6, yy, 0, 4 );

    //get new data
    float r = ByteBuffer.wrap(rr).order(ByteOrder.LITTLE_ENDIAN).getFloat();
    float p = ByteBuffer.wrap(pp).order(ByteOrder.LITTLE_ENDIAN).getFloat();
  
    //update curr
    rollCurr = r;
    pitchCurr = p;

    //update yArray
    for(int i=0; i<(width-1); i++)
    {
      rArray[i] = rArray[i+1];
      pArray[i] = pArray[i+1];
    }

    float FullSacle = 180;  //8000mg -> 78.48 ms2
  
    float r_scaled = ((float)-r)/FullSacle*(myEight3/2) + myEight3/2 + vertOffset3;
    rArray[myWidth3 - 1] = (short)r_scaled;
    
    float p_scaled = ((float)-p)/FullSacle*(myEight3/2) + myEight3/2 + vertOffset3;
    pArray[myWidth3 - 1] = (short)p_scaled;
    
}


//////////////////////////////////////////////////////////////////////////////////////
void demo3_plot()
{
  stroke(128, 128, 128);
  fill(16, 16, 16);
  rect(border3, vertOffset3 + border3, myWidth3 - 2*border3, myEight3 - border3, 7);
  
  
  for(int i=0; i<(myWidth3-1); i++)
  {
    stroke(255,0,0);
    strokeWeight(3);
    line(i, rArray[i], i+1, rArray[i+1]);
  }
    
  for(int i=0; i<(myWidth3-1); i++)
  {
    stroke(0,255,0);
    strokeWeight(3);
    line(i, pArray[i], i+1, pArray[i+1]);
  }
  
 
            
  textSize(32);
  textAlign(LEFT);
  int x, y;
  
  
  
  //compute horiz align based on a typical string
  x = (int)(myWidth3 - textWidth("Roll [mbar]:  120,4")) / 2;
  x = 60;
  
  
  //roll
  fill(255,0,0);
  y = vertOffset3 + 60;
  text("roll [deg]:  " + String.format("%.0f", rollCurr), x, y);  
  
  
  //pitch
  fill(0,255,0);
  y = vertOffset3 + 100;
  text("pitch [deg]:  " + String.format("%.0f", pitchCurr), x, y);
  
  
}
