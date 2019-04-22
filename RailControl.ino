#include <SoftwareSerial.h>
#include <NintendoExtensionCtrl.h>

const int PosMin = 0;
const int PosMax = 255;
const int ZeroPosMin = 128;
const int ZeroPosMax = 132;
const int MaxSpeed = 250;

typedef struct{
   int16_t steer;
   int16_t speed;
   //uint32_t crc;
} Serialcommand;

Nunchuk nunchuk; //A4/A5
SoftwareSerial SerialBoard(10, 11); //D10/D11

void setup() {
  Serial.begin(115200);
  nunchuk.begin();
  SerialBoard.begin(115200);
}

boolean success = false;
int lastJoyY = 127;
boolean firstConnection = true;
void loop() {
  Serialcommand command;
  int joyY = 127;
  boolean zButton = false;
  command.steer = 0;
  command.speed = 0;

  if(success == false)
  {
    success = nunchuk.connect(); //Connect controller
    //Serial.println("Connected");
    firstConnection = true;
  }
  else
  {
    success = nunchuk.update(); // Get new data from the controller
    //Serial.println("Updated");
    
    if(success == true)
    {
      joyY = nunchuk.joyY();
      zButton = nunchuk.buttonZ();
      //Serial.println("Readed");
      //Serial.println(joyY);
      //Serial.println(zButton);
      
      if(zButton == false)
      {
        firstConnection = true;
      }

      if(firstConnection == true &&
        zButton == true &&
        joyY >= ZeroPosMin &&
        joyY <= ZeroPosMax)
      {
        firstConnection = false;
      }
      
      if(firstConnection == false &&
        zButton == true &&
        (joyY < ZeroPosMin ||
        joyY > ZeroPosMax))
      {
        //Serial.println(joyY);
        command.speed = map(joyY, PosMin, PosMax, -1 * MaxSpeed, MaxSpeed);
        //Serial.println("Maped");
      }
    }
  }
  
  SerialBoard.print(command.steer);
  SerialBoard.print(" ");
  SerialBoard.println(command.speed);
  Serial.print(command.steer);
  Serial.print(" ");
  Serial.println(command.speed);
  delay(20);
}
