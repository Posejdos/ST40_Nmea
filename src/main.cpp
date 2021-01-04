#include <Arduino.h>
#include <SoftwareSerial9.h>

SoftwareSerial9 seatalk(12, 11); //rx, tx

unsigned char msg[4]; //message incoming
float speed;
int angle;

char nmeamsg[50];
char strspeed[8];
char checksum;

/*int hexadecimalToDecimal(char hexVal[]) 
{    
    int len = strlen(hexVal); 
      
    // Initializing base value to 1, i.e 16^0 
    int base = 1; 
      
    int dec_val = 0; 
      
    // Extracting characters as digits from last character 
    for (int i=len-1; i>=0; i--) 
    {    
        // if character lies in '0'-'9', converting  
        // it to integral 0-9 by subtracting 48 from 
        // ASCII value. 
        if (hexVal[i]>='0' && hexVal[i]<='9') 
        { 
            dec_val += (hexVal[i] - 48)*base; 
                  
            // incrementing base by power 
            base = base * 16; 
        } 
  
        // if character lies in 'A'-'F' , converting  
        // it to integral 10 - 15 by subtracting 55  
        // from ASCII value 
        else if (hexVal[i]>='A' && hexVal[i]<='F') 
        { 
            dec_val += (hexVal[i] - 55)*base; 
          
            // incrementing base by power 
            base = base*16; 
        } 
    } 
      
    return dec_val; 
} 
*/

int CalculateAngle(unsigned char *nums)
{
  char hex_x[3];
  sprintf(hex_x, "%X", nums[2]); //first byte

  char hex_y[2];
  sprintf(hex_y, "%X", nums[3]); //second byte

  char fullhex[10];
  sprintf(fullhex,"%s%s",hex_x,hex_y);

  unsigned long hexint;
  hexint = strtoul(fullhex, NULL, 16); //bytes to int

  int divided = hexint / 2;
  return (360 - divided);
} 

float CalculateSpeed(unsigned char *nums)
{
  float ones = nums[2]; //first byte
  float tenth = nums[3]; //second byte
  return ones + (tenth/10);
}

void setup() 
{
  Serial.begin(4800); //computer, NMEA
  while (!Serial) {;} // wait for serial port to connect. Needed for native USB port only
     
  Serial.println("Welcome.");
  seatalk.begin(4800); // set the data rate for the SoftwareSerial port
}

void loop()
{

  for (int i = 0; i < 4; i++) //4 times as 4 bytes in a frame
  {
    if (seatalk.available())
    {
      msg[i] = seatalk.read();
    }
  }

  if (msg[0] == 16) // ANGLE
  {
    angle = CalculateAngle(msg);
  }
  
  else if (msg[0] == 17) //SPEED
  {
    speed = CalculateSpeed(msg);
  }

  else //The first byte isn't 110 or 111
  {
    delay(200);
    return; //go to beggining of the loop, try again
  }

  dtostrf(speed, 3, 1, strspeed);
  sprintf(nmeamsg, "$WIMWV,%i,R,%s,N,A*", angle, strspeed);

  for (unsigned int i = 1; i < strlen(nmeamsg) - 1; i ++) // calculate checksum, leave $ and *
  {
    if (i == 1 && nmeamsg[i - 1] == '$')
    {
      checksum = nmeamsg[i];
      continue;
    }
    else
    {
      checksum = checksum xor nmeamsg[i];
    }
  }

  Serial.print(nmeamsg);
  Serial.println(checksum, HEX);
  delay(500);

}