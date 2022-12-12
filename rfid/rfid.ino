//Libraries
#include <Arduino.h>
#include <SPI.h>      //https://www.arduino.cc/en/reference/SPI
#include "MFRC522.h"  //https://github.com/miguelbalboa/rfid
//Constants
/*
Vcc                                 <-> 3V3 (or Vin(5V) depending on the module version)
RST (Reset)                         <-> D4
GND (Masse)                         <-> GND
MISO (Master Input Slave Output)    <-> D19
MOSI (Master Output Slave Input)    <-> D23
SCK (Serial Clock)                  <-> D18
SS/SDA (Slave select)               <-> D5
LED (DOOR)                          <-> D27
*/
#define SS_PIN 5
#define RST_PIN 4
#define LED_PIN 27

// #define nuidPICC
// #define WHITE_CARD
// #define BLUE_CARD_1
// #define BLUE_CARD_2

static int corr_checked = 0;
String cmd;

//Variables
// #ifdef nuidPICC
byte nuidPICC[4] = { 0, 0, 0, 0 };
byte nuidPICC_[4] = { 0, 0, 0, 0 };
// endif

// #ifdef WHITE_CARD
// byte white_card[4] = { 161, 64, 42, 29 };
// #endif

// #ifdef BLUE_CARD_1
// byte blue_card_1[4] = { 51, 190, 251, 182 };
// #endif

// #ifdef BLUE_CARD_2
// byte blue_card_2[4] = { 58, 40, 69, 41 };
// #endif

MFRC522::MIFARE_Key key;
MFRC522 rfid = MFRC522(SS_PIN, RST_PIN);

//Init timer
hw_timer_t *timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

void setup() {
  //Init Pin
  pinMode(LED_PIN, OUTPUT);

  //Init Serial USBS
  Serial.begin(115200);
  Serial.println(F("Initialize System"));

  //Init RFID with SPI
  SPI.begin();
  rfid.PCD_Init();
  //Show Firmware Version: Confirm RFID are connected to ESP32
  Serial.print(F("Reader :"));
  rfid.PCD_DumpVersionToSerial();
}



void loop() {
  Serial.println(F("\n================================================\nLet's scan!"));
  Serial.println(F("If you want to change card. Let's input '0' in anytime!"));
  cmd = Serial.readString();
  cmd.trim();
  // Serial.println(cmd);
  while(cmd == "0")
  {
      changeCard();
      break;
  }
  cmd = " ";
  runRFID();
}
void changeCard()
{
  
  while(nuidPICC_[0] != nuidPICC[0] && nuidPICC_[1] != nuidPICC[1] && nuidPICC_[2] != nuidPICC[2] && nuidPICC_[3] != nuidPICC[3])
  {
    if (rfid.PICC_IsNewCardPresent())
      if (rfid.PICC_ReadCardSerial())
      {
        nuidPICC_[0] = rfid.uid.uidByte[0];
        nuidPICC_[1] = rfid.uid.uidByte[1];
        nuidPICC_[2] = rfid.uid.uidByte[2];
        nuidPICC_[3] = rfid.uid.uidByte[3];
      }

    if(nuidPICC_[0] != nuidPICC[0] && nuidPICC_[1] != nuidPICC[1] && nuidPICC_[2] != nuidPICC[2] && nuidPICC_[3] != nuidPICC[3])
      Serial.println(F("Let's scan recent card!"));
  }

  while(nuidPICC_[0] == nuidPICC[0] && nuidPICC_[1] == nuidPICC[1] && nuidPICC_[2] == nuidPICC[2] && nuidPICC_[3] == nuidPICC[3])
  {
    if (rfid.PICC_IsNewCardPresent())
      if (rfid.PICC_ReadCardSerial())
      {
        nuidPICC[0] = rfid.uid.uidByte[0];
        nuidPICC[1] = rfid.uid.uidByte[1];
        nuidPICC[2] = rfid.uid.uidByte[2];
        nuidPICC[3] = rfid.uid.uidByte[3];
      }

    if(nuidPICC_[0] == nuidPICC[0] && nuidPICC_[1] == nuidPICC[1] && nuidPICC_[2] == nuidPICC[2] && nuidPICC_[3] == nuidPICC[3])
      Serial.println(F("Let's scan new card!"));
  }
  Serial.println(F("Changed to New Card!"));
}
//Read NUID and Run features.
void runRFID() {
  if(nuidPICC[0] == 0 && nuidPICC[1] == 0 && nuidPICC[2] == 0 && nuidPICC[3] == 0)
  {
    Serial.println(F("Let's scan first card!"));
    if (rfid.PICC_IsNewCardPresent())
      if (rfid.PICC_ReadCardSerial())
      {
        nuidPICC[0] = rfid.uid.uidByte[0];
        nuidPICC[1] = rfid.uid.uidByte[1];
        nuidPICC[2] = rfid.uid.uidByte[2];
        nuidPICC[3] = rfid.uid.uidByte[3];
      }
  }
  if (!rfid.PICC_IsNewCardPresent())
    return;
  if (!rfid.PICC_ReadCardSerial())
    return;

  if(rfid.uid.uidByte[0] == nuidPICC[0] && rfid.uid.uidByte[1] == nuidPICC[1] && rfid.uid.uidByte[2] == nuidPICC[2] && rfid.uid.uidByte[3] == nuidPICC[3])
  {
    for (byte i = 0; i < 4; i++) {
      Serial.print(nuidPICC[i] < 0x10 ? " 0" : " ");
      Serial.print(nuidPICC[i], DEC);
    }
    Serial.println(F("\nCorrect card -> The door opened!"));
    digitalWrite(LED_PIN, HIGH);
    delay(5000);
    Serial.println(F("The door closed!"));
  }

  else {
    Serial.println(F("Incorrect card -> The door is still closed!!!"));
  }
  digitalWrite(LED_PIN, LOW);

  
}
