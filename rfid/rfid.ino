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

#define WHITE_CARD
#define BLUE_CARD_1
// #define BLUE_CARD_2

//Variables
byte nuidPICC[4] = { 0, 0, 0, 0 };
#ifdef WHITE_CARD
byte white_card[4] = { 161, 64, 42, 29 };
#endif

#ifdef BLUE_CARD_1
byte blue_card_1[4] = { 51, 190, 251, 182};
#endif

#ifdef BLUE_CARD_2
byte blue_card_2[4] = { 58, 40, 69, 41 };
#endif

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
  //Show infomation
  Serial.print(F("Reader :"));
  rfid.PCD_DumpVersionToSerial();
}



void loop() {
  // readRFID();
  runRFID();
}

//Read NUID and Run features.
void runRFID() {
  // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
  if (!rfid.PICC_IsNewCardPresent())
    return;

  // Verify if the NUID has been readed
  if (!rfid.PICC_ReadCardSerial())
    return;

  if (
#ifdef WHITE_CARD
    (rfid.uid.uidByte[0] == white_card[0] 
    && rfid.uid.uidByte[1] == white_card[1] 
    && rfid.uid.uidByte[2] == white_card[2] 
    && rfid.uid.uidByte[3] == white_card[3])
#endif
    
#ifdef BLUE_CARD_1
    ||
    (rfid.uid.uidByte[0] == blue_card_1[0] 
    && rfid.uid.uidByte[1] == blue_card_1[1] 
    && rfid.uid.uidByte[2] == blue_card_1[2] 
    && rfid.uid.uidByte[3] == blue_card_1[3])
#endif
    
#ifdef BLUE_CARD_2
    ||
    (rfid.uid.uidByte[0] == blue_card_2[0] 
    && rfid.uid.uidByte[1] == blue_card_2[1] 
    && rfid.uid.uidByte[2] == blue_card_2[2] 
    && rfid.uid.uidByte[3] == blue_card_2[3])
#endif
    ) 
  {
    Serial.println(F("Correct card -> The door opened!"));
    digitalWrite(LED_PIN, HIGH);
    delay(5000);
    Serial.println(F("The door closed!"));
  }

  else {
    Serial.println(F("Incorrect card -> The door is still closed!!!"));
  }
  digitalWrite(LED_PIN, LOW);
}