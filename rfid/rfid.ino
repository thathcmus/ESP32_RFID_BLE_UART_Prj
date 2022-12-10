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
LED (DOOR)                          <-> D2
*/
#define SS_PIN  5
#define RST_PIN 4
#define LED_PIN 2

//Variables
byte nuidPICC[4] = { 0, 0, 0, 0 };
byte nuid_saved_1[4] = { 1, 2, 3, 4 };
MFRC522::MIFARE_Key key;
MFRC522 rfid = MFRC522(SS_PIN, RST_PIN);

//Init timer
hw_timer_t* timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;


// Interrupt Handler
void IRAM_ATTR onTimer() {   
  portENTER_CRITICAL_ISR(&timerMux);
  digitalWrite(LED_PIN, LOW);
  if(timer)
  {
    timerEnd(timer);
    timer = NULL;
  }
  portEXIT_CRITICAL_ISR(&timerMux);
}

void setup() {
  //Init Pin
  pinMode(LED_PIN, OUTPUT);
  
  //1us each time
  timer = timerBegin(0, 80, true);
  //Init Handler Timer
  timerAttachInterrupt(timer, &onTimer, true);
  //Init interrupt timer 5s
  timerWrite(timer, 5000000);

  //Timer: https://espressif-docs.readthedocs-hosted.com/projects/arduino-esp32/en/latest/api/timer.html

  //Init Serial USB
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

// Function to Check NUID of card
void readRFID(void) {
  ////Read RFID card
  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }
  // Look for new 1 cards
  if (!rfid.PICC_IsNewCardPresent())
    return;
  // Verify if the NUID has been readed
  if (!rfid.PICC_ReadCardSerial())
    return;
  // Store NUID into nuidPICC array
  for (byte i = 0; i < 4; i++) {
    nuidPICC[i] = rfid.uid.uidByte[i];
  }
  Serial.print(F("RFMFRC522ID In dec: "));
  printDec(rfid.uid.uidByte, rfid.uid.size);
  Serial.println();
  // Halt PICC
  rfid.PICC_HaltA();
  // Stop encryption on PCD
  rfid.PCD_StopCrypto1();
}

//Read NUID and Run features.
void runRFID()
{
  // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
  if ( ! rfid.PICC_IsNewCardPresent())
    return;

  // Verify if the NUID has been readed
  if ( ! rfid.PICC_ReadCardSerial())
    return;

  if (rfid.uid.uidByte[0] == nuid_saved_1[0] &&
    rfid.uid.uidByte[1] == nuid_saved_1[1] &&
    rfid.uid.uidByte[2] == nuid_saved_1[2] &&
    rfid.uid.uidByte[3] == nuid_saved_1[3] ) {
      Serial.println(F("Correct card. The door opened!"));
      digitalWrite(LED_PIN, HIGH);
      timerStart(timer);
    }

  else{
    Serial.println(F("Incorect card. The door still close!!"));
    }
}
/**
 		Helper routine to dump a byte array as hex values to Serial.
*/
void printHex(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}
/**
 		Helper routine to dump a byte array as dec values to Serial.
*/
void printDec(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], DEC);
  }
}

//https://deviot.vn/tutorials/esp32.66047996/ngat-timer.67695749