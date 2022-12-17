/* Includes ----------------------------------------------------------- */
#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <SPI.h>     
#include "MFRC522.h"  
#include <EEPROM.h>

/* Private defines ---------------------------------------------------- */
#define SERVICE_UUID                        "844fc6d6-1c7e-461a-abb9-3a9a9fdd597e" 
#define CHARACTERISTIC_UUID_PASS            "909aa990-1e33-4bf6-bfbd-11efd5b571a4"
#define CHARACTERISTIC_UUID_CHECK           "80a80bef-ea4c-426f-885a-5c0a34757a54"
#define CHARACTERISTIC_UUID_VERIFY          "0515e27d-dd91-4f96-9452-5f43649c1819"
#define CHARACTERISTIC_UUID_CHANGE_PASS     "688091db-1736-4179-b7ce-e42a724a6a68"
#define SS_PIN 5
#define RST_PIN 4
#define LED  2

/* Private macros ----------------------------------------------------- */

/* Public variables --------------------------------------------------- */
BLEServer *pServer = NULL;
BLECharacteristic *pCharacteristic;
bool deviceConnected = false;
bool oldDeviceConnected = false;
int ble_connected = 0;
int ble_disconnect = 0;
// 0.0 is incorrect, 1.1 is correct
float txValue = 0.0; 

// Password for the door
char pass_correct[] = "Iot2022";
String pass_test1 = "Iot2022";
String pass_test2 = "Iot2023";
char passwordTemp[] = "";
char txString[8];
String cmd;

// RFID
byte nuidPICC[4];
bool have_change = false;
bool have_scan = false; 
MFRC522::MIFARE_Key key;
MFRC522 rfid = MFRC522(SS_PIN, RST_PIN);

int eepromAddr1 = 0, eepromAddr2 = 10, eepromAddr3 = 11, eepromAddr4 = 12, eepromAddr5 = 13;

/* Private function prototypes ---------------------------------------- */

/* Function definitions ----------------------------------------------- */
/**
 * @brief BLE server callback
 */
class MyServerCallbacks : public BLEServerCallbacks
{
  void onConnect(BLEServer *pServer)
  {
    deviceConnected = true;
  };

  void onDisconnect(BLEServer *pServer)
  {
    deviceConnected = false;
  }
};

/**
 * @brief BLE characteristic callback
 */
class MyCallbacks : public BLECharacteristicCallbacks
{
  void onRead(BLECharacteristic *pCharacteristic)
  {
    char txString[8];                
    dtostrf(txValue, 1, 1, txString); 
    pCharacteristic->setValue(txString);
  }

  void onWrite(BLECharacteristic *pCharacteristic)
  {
    BLEUUID uuid = pCharacteristic->getUUID();
    std::string rxValue = pCharacteristic->getValue();

    // CHARACTERISTIC_UUID_VERIFY    
    if (uuid.toString() == CHARACTERISTIC_UUID_VERIFY)
    {
      if (pass_correct == rxValue)
      {
        txValue = 1.1;
        dtostrf(txValue, 1, 1, txString);
        pCharacteristic->setValue(txString); // Sending  message
        pCharacteristic->notify();           // Send the value to the app!
      }
      else
      {
        txValue = 0.0;
        dtostrf(txValue, 1, 1, txString);
        pCharacteristic->setValue(txString); // Sending  message
        pCharacteristic->notify();           // Send the value to the app!
      }
    }

    // CHARACTERISTIC_UUID_CHANGE_PASS
    if (uuid.toString() == CHARACTERISTIC_UUID_CHANGE_PASS)
    {
      char char_array[rxValue.length()];
      for (int i = 0; i < rxValue.length(); i++)
      {
        char_array[i] = rxValue[i];
      }
      char_array[rxValue.length()] = NULL;

      for (int i = 0; char_array[i] != NULL; i++)
      {
        pass_correct[i] = char_array[i];
        pass_correct[i + 1] = NULL;
      }
    }

    // CHARACTERISTIC_UUID_PASS
    if (uuid.toString() == CHARACTERISTIC_UUID_PASS)
    {
      if (pass_correct == rxValue)
      {
        txValue = 1.1;
        dtostrf(txValue, 1, 1, txString);
        pCharacteristic->setValue(txString); // Sending  message
        pCharacteristic->notify();           // Send the value to the app!
        digitalWrite(LED, HIGH);
        delay(5000);
        digitalWrite(LED, LOW);
      }
      else
      {
        txValue = 0.0;
        dtostrf(txValue, 1, 1, txString);
        pCharacteristic->setValue(txString); // Sending  message
        pCharacteristic->notify();           // Send the value to the app!
        digitalWrite(LED, LOW);
      }
    }
  }
};

/**
 * @brief Type your password to open door
 */
void getPassToOPenDoor()
{
  Serial.println();
  Serial.println("=================================================================");
  Serial.print("Type Your Password: ");
  for (int i = 1; i <= 3; i++)
  {
    while (Serial.available() == 0)
    {
    } // wait for data available
    cmd = Serial.readString();
    cmd.trim();
    cmd.toCharArray(passwordTemp, cmd.length() + 1);
    Serial.println(passwordTemp);
    if (strcmp(pass_correct, passwordTemp))
    {
      Serial.print("\t\tWRONG PASSWORD: ");
      Serial.println(i);
      if (i < 3)
      {
        Serial.print("Type Your Password Again: ");
      }
      else
      {
        Serial.println("=================================================================");
        Serial.println("\t\tWARNING!!!");
      }
    }
    else
    {
      digitalWrite(LED, HIGH);
      Serial.println("\t\tYOUR DOOR IS OPENED");
      break;
    }
  }
  dashboard();
}

/**
 * @brief Type your new password to change password
 */
void changePass()
{
  Serial.println();
  Serial.println("=================================================================");
  Serial.print("Type your old Password: ");
  for (int i = 1; i <= 3; i++)
  {
    while (Serial.available() == 0)
    {
    }
    cmd = Serial.readString();
    cmd.trim();
    cmd.toCharArray(passwordTemp, cmd.length() + 1);
    Serial.println(cmd);
    if (strcmp(passwordTemp, pass_correct))
    {
      Serial.print("\t\tWRONG PASSWORD: ");
      Serial.println(i);
      if (i < 3)
      {
        Serial.println("=================================================================");
        Serial.print("Type your old Password again: ");
      }
      else
      {
        Serial.println("=================================================================");
        Serial.println("\t\tWARNING!!!");
      }
    }
    else
    {
      Serial.print("Type your new Password: ");
      while (Serial.available() == 0)
      {
      }
      cmd = Serial.readString();
      cmd.trim();
      cmd.toCharArray(passwordTemp, cmd.length() + 1);
      Serial.println(passwordTemp);
      strcpy(pass_correct, passwordTemp);
      String rString = String(pass_correct);

      EEPROM.writeString(eepromAddr1, rString);
      EEPROM.commit();

      rString = EEPROM.readString(eepromAddr1);
      Serial.println("Read from EEPROM");
      Serial.print("Str:");
      Serial.println(rString); 

      char char_array[rString.length()];
      for (int i = 0; i < rString.length(); i++)
      {
        char_array[i] = rString[i];
      }
        char_array[rString.length()] = NULL;

      for (int i = 0; char_array[i] != NULL; i++)
      {
        pass_correct[i] = char_array[i];
        pass_correct[i + 1] = NULL;
      }
      Serial.println("\t\tCHANGE YOUR NEW PASSWORD SUCCESS");
      break;
    }
  }
  dashboard();
}

/**
 * @brief Close the door
 */
void closeDoor()
{
  Serial.println();
  Serial.println("=================================================================");
  digitalWrite(LED, LOW);
  Serial.println("\t\tYOUR DOOR IS CLOSED");
  dashboard();
}

/**
 * @brief Check id from card, if correct, it will open door
 */
void runRFID() {
  if (rfid.PICC_IsNewCardPresent()) {  // new tag is available
    if (rfid.PICC_ReadCardSerial()) {  // NUID has been readed

      // print UID in Serial Monitor in the hex format
      Serial.println();
      Serial.println("=================================================================");
      Serial.println("\t\tREVEIVED CARD");
      Serial.print("\t\tID:");
      for (int i = 0; i < rfid.uid.size; i++) {
        Serial.print(rfid.uid.uidByte[i] < 0x10 ? " 0" : " ");
        Serial.print(rfid.uid.uidByte[i], HEX);
      }
      Serial.println();

      rfid.PICC_HaltA();       // halt PICC
      rfid.PCD_StopCrypto1();  // stop encryption on PCD

      if (rfid.uid.uidByte[0] == nuidPICC[0] && rfid.uid.uidByte[1] == nuidPICC[1] && rfid.uid.uidByte[2] == nuidPICC[2] && rfid.uid.uidByte[3] == nuidPICC[3]) {
        Serial.println("\t\tVERIFY SUCCESS! OPEN DOOR");
        digitalWrite(LED, HIGH);
        delay(5000);
        Serial.println("\t\tCLOSED DOOR");
      } else {
        Serial.println("\t\tVERIFY FAILED! CAN'T OPEN DOOR");
      }
      digitalWrite(LED, LOW);
    }
     dashboard();
  }
}

/**
 * @brief Check id from card, if correct, it will allow change new card
 */
void changeCard() {
  Serial.println();
  Serial.println("=================================================================");
  Serial.println("\t\tSCANNING TO VERIFY CORRECT CARD ........");
  have_change = true;
  while(have_change){
     if (rfid.PICC_IsNewCardPresent()) {  // new tag is available
    if (rfid.PICC_ReadCardSerial()) {  // NUID has been readed

      // print UID in Serial Monitor in the hex format
      Serial.print("\t\tID:");
      for (int i = 0; i < rfid.uid.size; i++) {
        Serial.print(rfid.uid.uidByte[i] < 0x10 ? " 0" : " ");
        Serial.print(rfid.uid.uidByte[i], HEX);
      }
      Serial.println();

      rfid.PICC_HaltA();       // halt PICC
      rfid.PCD_StopCrypto1();  // stop encryption on PCD

      if (rfid.uid.uidByte[0] == nuidPICC[0] && rfid.uid.uidByte[1] == nuidPICC[1] && rfid.uid.uidByte[2] == nuidPICC[2] && rfid.uid.uidByte[3] == nuidPICC[3]) {
        Serial.println("\t\tCORRECT CARD! CHANGING TO NEW CARD ........");
        have_scan = true;
      } else {
        Serial.println("\t\tINCORRECT CARD! CAN'T CHANGE TO NEW CARD !!!");
        have_scan = false;
      }
      have_change = false;
    }
  }
  }

  while(have_scan)
  {
    if (rfid.PICC_IsNewCardPresent()) {  // new tag is available
    if (rfid.PICC_ReadCardSerial()) {  // NUID has been readed

      // print UID in Serial Monitor in the hex format
      Serial.print("\t\tID:");
      for (int i = 0; i < rfid.uid.size; i++) {
        Serial.print(rfid.uid.uidByte[i] < 0x10 ? " 0" : " ");
        Serial.print(rfid.uid.uidByte[i], HEX);
        nuidPICC[i] = rfid.uid.uidByte[i];       
      }
      Serial.println();

      EEPROM.write(eepromAddr2, nuidPICC[0]);
      EEPROM.write(eepromAddr3, nuidPICC[1]);
      EEPROM.write(eepromAddr4, nuidPICC[2]);
      EEPROM.write(eepromAddr5, nuidPICC[3]);
      EEPROM.commit();

      byte b[4];
      b[0] = EEPROM.read(eepromAddr2);
      b[1] = EEPROM.read(eepromAddr3);
      b[2] = EEPROM.read(eepromAddr4);
      b[3] = EEPROM.read(eepromAddr5);
      for (int i = 0; i < 4; i++)
      {
        nuidPICC[i] = b[i];
      }

      rfid.PICC_HaltA();       // halt PICC
      rfid.PCD_StopCrypto1();  // stop encryption on PCD
      Serial.println("\t\tDONE CHANGE NEW CARD ***");  
      have_scan = false;    
    }
  }
  }
  dashboard();
}

/* Set up ------------------------------------------------------------- */
void setup()
{
  Serial.begin(115200);
  EEPROM.begin(512);

  // EEPROM.writeString(eepromAddr2, pass_test1);
  // EEPROM.commit();
  ////
  // byte nuidPICC[4] = { 0x93, 0xAC, 0x76, 0xA3 };
  // EEPROM.write(eepromAddr2, 0x93);
  // EEPROM.write(eepromAddr3, 0xAC);
  // EEPROM.write(eepromAddr4, 0x76);
  // EEPROM.write(eepromAddr5, 0xA3);
  // EEPROM.commit();

  byte b[4];
  Serial.print("Byte:");
  b[0] = EEPROM.read(eepromAddr2);
  b[1] = EEPROM.read(eepromAddr3);
  b[2] = EEPROM.read(eepromAddr4);
  b[3] = EEPROM.read(eepromAddr5);
  for (int i = 0; i < 4; i++)
  {
    nuidPICC[i] = b[i];
  }
  // Serial.println(nuidPICC[0]);
  // Serial.println(nuidPICC[1]);
  // Serial.println(nuidPICC[2]);
  // Serial.println(nuidPICC[3]);
  ////

  String rString = EEPROM.readString(eepromAddr1);
  Serial.println("Read from EEPROM");
  Serial.print("Str:");
  Serial.println(rString); 

  char char_array[rString.length()];
  for (int i = 0; i < rString.length(); i++)
  {
    char_array[i] = rString[i];
  }
  char_array[rString.length()] = NULL;

  for (int i = 0; char_array[i] != NULL; i++)
  {
    pass_correct[i] = char_array[i];
    pass_correct[i + 1] = NULL;
  }
 
  pinMode(LED, OUTPUT);

  // Init RFID with SPI
  SPI.begin();
  rfid.PCD_Init();  

  // Create the BLE Device
  BLEDevice::init("ESP32_OpenDoor"); // Give it a name

  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create a BLE Characteristic to check correct password
  pCharacteristic = pService->createCharacteristic( CHARACTERISTIC_UUID_CHECK,
                                                    BLECharacteristic::PROPERTY_READ | 
                                                    BLECharacteristic::PROPERTY_NOTIFY);
  pCharacteristic->addDescriptor(new BLE2902());
  pCharacteristic->setCallbacks(new MyCallbacks());

  // Create a BLE Characteristic to send password
  pCharacteristic = pService->createCharacteristic( CHARACTERISTIC_UUID_PASS,
                                                    BLECharacteristic::PROPERTY_WRITE);
  pCharacteristic->setCallbacks(new MyCallbacks());

  // Create a BLE Characteristic to verify password
  pCharacteristic = pService->createCharacteristic( CHARACTERISTIC_UUID_VERIFY,
                                                    BLECharacteristic::PROPERTY_WRITE);
  pCharacteristic->setCallbacks(new MyCallbacks());

  // Create a BLE Characteristic to change password
  pCharacteristic = pService->createCharacteristic( CHARACTERISTIC_UUID_CHANGE_PASS,
                                                    BLECharacteristic::PROPERTY_WRITE);
  pCharacteristic->setCallbacks(new MyCallbacks());

  // Start the service
  pService->start();

  // Start advertising
  pServer->getAdvertising()->start();
  dashboard();
}

/**
 * @brief Display as a menu to choose some features
 */
void dashboard()
{
  Serial.println("================================================================="); 
  Serial.println("\t\t1.Type password to open your door");
  Serial.println("\t\t2.Change your password");
  Serial.println("\t\t3.Close your door");
  Serial.println("\t\t4.Change RFID card");
  Serial.print("\t\tEnter your choose:");
}

/* Main loop ---------------------------------------------------------- */
void loop()
{
  //============================================RFID=========================================//
  runRFID(); 
  //============================================RFID=========================================//
  
  //============================================UART=========================================//
  cmd = Serial.readString();
  cmd.trim();
  Serial.print(" " + cmd);
  
  if (cmd == "1") // Case 1: type your password to open door
  {
    getPassToOPenDoor();
  }
  else if (cmd == "2") // Case 2: type your new password to change password
  {
    changePass();
  }
  else if (cmd == "3") // Case 3: close the door
  {
    closeDoor();
  }
    else if (cmd == "4") // Case 3: close the door
  {
   changeCard();
  }
  //============================================UART=========================================//

  //============================================BLE=========================================//
  // BLE connected
  if (deviceConnected)
  {
    // TO DO
    ble_connected++;
    ble_disconnect = 0;
  }
  delay(1000);

  if(ble_connected == 1)
  {
    Serial.println();
    Serial.println("=================================================================");
    Serial.println("\t\tCONNECTED DEVICE WITH APP BY BLE !!!");
    delay(2000);
    dashboard();
  }

  if(ble_disconnect == 1)
  {
    Serial.println();
    Serial.println("=================================================================");
    Serial.println("\t\tDISCONNECTED DEVICE WITH APP BY BLE !!!");
    ble_disconnect++;
    delay(2000);
    dashboard();
  }

  // BLE disconnecting
  if (!deviceConnected && oldDeviceConnected)
  {
    // restart advertising
    pServer->startAdvertising(); 
    oldDeviceConnected = deviceConnected;
    ble_connected = 0;
    ble_disconnect++;
  }

  // BLE connecting
  if (deviceConnected && !oldDeviceConnected)
  {
    oldDeviceConnected = deviceConnected;
  }
  //============================================BLE=========================================//
}
/* End of file -------------------------------------------------------- */