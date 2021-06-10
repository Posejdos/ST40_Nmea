#include <Arduino.h>
#include <SoftwareSerial9.h>

SoftwareSerial9 seatalk(4, 11); //rx, tx

unsigned char msg[4]; //message incoming
uint16_t cmd_check; //command check
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
  if (seatalk.available())
  {
    cmd_check = seatalk.read(); //first 9 bit "byte", need to check if 9th bit is high

    for (int i = 1; i <= 3; i++) //bytes 1,2,3 - I can skip the 9th bit here
    {
      msg[i] = seatalk.read();
    }
    //Use cmd_check to check the 9th bit
    //Use msg[0] as a char for parsing
    msg[0] = cmd_check;
  }

  if(!((cmd_check >> 9) & 1)) //check the 9th bit of the 1st "byte", if not 1 then return to start of loop
  {
    return;
  }

  switch (msg[0])
  {
    case 16:
      angle = CalculateAngle(msg);
      break;

    case 17:
      speed = CalculateSpeed(msg);
      break;
    
    default:
      return;
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

  cmd_check = 0; //reset the command "flag"
}