
short[] xArray;
short[] yArray;
short[] zArray;

float xCurr = 0;
float yCurr = 0;
float zCurr = 0;

int myWidth;
int myEight;
int vertOffset;

/*
          -----------------> width
          |
          |
          |
          |
          |
          |
          |
          V   height
*/



//////////////////////////////////////////////////////////////////////////////////////
void demo1_init()
{
  myWidth = width;
  myEight = height / 3;
  vertOffset = 0;
  
  //inizializza array con i punti da plottare
  xArray = new short[myWidth];  
  yArray = new short[myWidth];
  zArray = new short[myWidth];
  for(int i=0; i<myWidth; i++)
  {
    xArray[i] = (short)((myEight/2) + vertOffset);
    yArray[i] = (short)((myEight/2) + vertOffset);
    zArray[i] = (short)((myEight/2) + vertOffset);
  }
}



//////////////////////////////////////////////////////////////////////////////////////
void demo1_CB(byte[] data)
{
    //  | timeStamp(2) | acc1(4) | acc2(4) | acc3(4) |
  
    // get each set of two bytes
    byte[] xb = new byte[4];
    byte[] yb = new byte[4];
    byte[] zb = new byte[4];
  
    System.arraycopy( data, 2, xb, 0, 4 );
    System.arraycopy( data, 6, yb, 0, 4 );
    System.arraycopy( data, 10, zb, 0, 4 );

    //get new data
    float x = ByteBuffer.wrap(xb).order(ByteOrder.LITTLE_ENDIAN).getFloat();
    float y = ByteBuffer.wrap(yb).order(ByteOrder.LITTLE_ENDIAN).getFloat();
    float z = ByteBuffer.wrap(zb).order(ByteOrder.LITTLE_ENDIAN).getFloat();
  
    //update curr
    xCurr = x;
    yCurr = y;
    zCurr = z;

    //update yArray
    for(int i=0; i<(width-1); i++)
    {
      xArray[i] = xArray[i+1];
      yArray[i] = yArray[i+1];
      zArray[i] = zArray[i+1];
    }

    float FullSacle = 17.4532;  //8000mg -> 78.48 ms2
  
    float x_scaled = ((float)-x)/FullSacle*(myEight/2) + myEight/2 + vertOffset;
    xArray[myWidth - 1] = (short)x_scaled;
    
    float y_scaled = ((float)-y)/FullSacle*(myEight/2) + myEight/2 + vertOffset;
    yArray[myWidth - 1] = (short)y_scaled;
    
    float z_scaled = ((float)-z)/FullSacle*(myEight/2) + myEight/2 + vertOffset;
    zArray[myWidth - 1] = (short)z_scaled;
}


//////////////////////////////////////////////////////////////////////////////////////
void demo1_plot()
{
  int border = 10;
  stroke(128, 128, 128);
  fill(16, 16, 16);
  rect(border, border, myWidth - 2*border, myEight - 2*border, 7);
  
  
  for(int i=0; i<(myWidth-1); i++)
  {
    stroke(255,0,0);
    strokeWeight(3);
    line(i, xArray[i], i+1, xArray[i+1]);
  }
    
  for(int i=0; i<(myWidth-1); i++)
  {
    stroke(0,255,0);
    strokeWeight(3);
    line(i, yArray[i], i+1, yArray[i+1]);
  }
  
  for(int i=0; i<(myWidth-1); i++)
  {
    stroke(255,0,255);
    strokeWeight(3);
    line(i, zArray[i], i+1, zArray[i+1]);
  }
    
            
  textSize(32);
  textAlign(RIGHT);
  
  fill(255,0,0);
  text("x-acc [ms2]", 220, 60);         text(xCurr, 400, 60);   
  fill(0,255,0);
  text("y-acc [ms2]", 220, 100);        text(yCurr, 400, 100);   
  fill(255,0,255);
  text("z-acc [ms2]", 220, 140);        text(zCurr, 400, 140);
}
