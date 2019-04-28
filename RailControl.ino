#include <SoftwareSerial.h>
#include <NintendoExtensionCtrl.h>

const int PosMin = 0;
const int PosMax = 255;
const int ZeroPosMin = 128;
const int ZeroPosMax = 132;
const int MaxSpeed = 250;

typedef struct MsgToHoverboard_t{
  unsigned char SOM;  // 0x02
  unsigned char len;  // len is len of ALL bytes to follow, including CS
  unsigned char cmd;  // 'W'
  unsigned char code; // code of value to write
  int16_t base_pwm;   // absolute value ranging from -1000 to 1000 .. base_pwm plus/minus steer is the raw PWM value
  int16_t steer;      // absolute value ranging from -1000 to 1000 .. wether steer is added or substracted depends on the side R/L
  unsigned char CS;   // checksumm
};

typedef union UART_Packet_t{
  MsgToHoverboard_t msgToHover;
  byte UART_Packet[sizeof(MsgToHoverboard_t)];
};

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
  UART_Packet_t ups;
  int joyY = 127;
  boolean zButton = false;
  int16_t base_pwm = 0;

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
        base_pwm = map(joyY, PosMin, PosMax, -1 * MaxSpeed, MaxSpeed);
        //Serial.println("Maped");
      }
    }
  }
  
  ups.msgToHover.SOM = 2 ;  // PROTOCOL_SOM; //Start of Message;
  ups.msgToHover.len = 7;   // payload + SC only
  ups.msgToHover.cmd  = 'W'; // PROTOCOL_CMD_WRITEVAL;  // Write value
  ups.msgToHover.code = 0x07; // speed data from params array
  ups.msgToHover.base_pwm = base_pwm;
  ups.msgToHover.steer = 0;
  ups.msgToHover.CS = 0;

  for (int i = 0; i < ups.msgToHover.len; i++){
    ups.msgToHover.CS -= ups.UART_Packet[i+1];
  }

  SerialBoard.write(ups.UART_Packet, sizeof(UART_Packet_t));
  
  if (SerialBoard.available()) {
    Serial.write(SerialBoard.read());
  }
  Serial.println(base_pwm);
  delay(20);
}
