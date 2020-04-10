

float tCurr = 0;
float pCurr = 0;


int myWidth2;
int myEight2;
int vertOffset2;
int border2 = 10;



//////////////////////////////////////////////////////////////////////////////////////
void demo2_init()
{
  myWidth2 = width;
  myEight2 = height / 3 - 2 * border2;
  vertOffset2 = height / 3;  
}


//////////////////////////////////////////////////////////////////////////////////////
void demo2_CB(byte[] data)
{
    //  | timeStamp(2) | temp(4) | press(4) 
    
    // get each set of two bytes
    byte[] tt = new byte[4];
    byte[] pp = new byte[4];
  
    System.arraycopy( data, 2, tt, 0, 4 );
    System.arraycopy( data, 6, pp, 0, 4 );

    //get new data
    float temp = ByteBuffer.wrap(tt).order(ByteOrder.LITTLE_ENDIAN).getFloat();
    float press = ByteBuffer.wrap(pp).order(ByteOrder.LITTLE_ENDIAN).getFloat();
  
    //update curr
    tCurr = temp;
    pCurr = press;
}


//////////////////////////////////////////////////////////////////////////////////////
void demo2_plot()
{
  stroke(128, 128, 128);
  fill(16, 16, 16);
  rect(border2, vertOffset2 + border2, myWidth2 - 2*border2, myEight2 - border2, 7);
  
    
  textSize(60);
  textAlign(LEFT);
  int x, y;
  
  //compute horiz align based on a typical string
  x = (int)(myWidth2 - textWidth("P [mbar]:  1234,56")) / 2;
  
  //temperature
  fill(255,0,0);
  y = vertOffset2 + 130;
  text("T [\u00B0C]:  " + String.format("%.1f", tCurr), x, y);
  
  //pressure
  fill(0,255,0);
  y = vertOffset2 + 250;
  text("P [mbar]:  " + String.format("%.1f", pCurr), x, y); 
}
