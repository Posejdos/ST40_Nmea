#include <Arduino.h>
#include <SoftwareSerial9.h>

SoftwareSerial9 seatalk(4, 11); //rx, tx

unsigned char msg[4]; //message incoming
float speed;
int angle;

char nmeamsg[50];
char strspeed[8];
char checksum;

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
  return (divided);
} 

float CalculateSpeed(unsigned char *nums)
{
  float ones = nums[2] & 0x7F; //first byte
  //If this is still wrong, you're probably getting wrong units
  float tenth = nums[3]; //second byte
  return ones + (tenth/10);
}

void setup() 
{
  Serial.begin(4800); //computer, NMEA
  seatalk.begin(4800); // set the data rate for the SoftwareSerial port

  pinMode(5, OUTPUT); digitalWrite(5, HIGH); //needed for seatalk, 1 means "I'm listening"
  pinMode(4, INPUT); //test seatalk input
  pinMode(3, INPUT);
  pinMode(2, INPUT); //jumper from tx
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
  Serial.print(checksum, HEX);
  Serial.println("<CR><LF>");
  delay(500);
}