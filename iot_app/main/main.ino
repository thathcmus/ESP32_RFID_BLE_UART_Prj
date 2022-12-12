#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
//Include for RFID
#include <SPI.h>      //https://www.arduino.cc/en/reference/SPI
#include "MFRC522.h"  //https://github.com/miguelbalboa/rfid



BLEServer *pServer = NULL;
BLECharacteristic *pCharacteristic;

bool deviceConnected = false;
bool oldDeviceConnected = false;
float txValue = 0.0; // 0.0 is incorrect, 1.1 is correct
const int LED = 2;   // Could be different depending on the dev board. I used the DOIT ESP32 dev board.
// Password for the door
char pass_correct[] = "Tung123";
String cmd;
char passwordTemp[] = "";
char txString[8];

#define SERVICE_UUID "844fc6d6-1c7e-461a-abb9-3a9a9fdd597e" // UART service UUID
#define CHARACTERISTIC_UUID_PASS "909aa990-1e33-4bf6-bfbd-11efd5b571a4"
#define CHARACTERISTIC_UUID_CHECK "80a80bef-ea4c-426f-885a-5c0a34757a54"
#define CHARACTERISTIC_UUID_VERIFY "0515e27d-dd91-4f96-9452-5f43649c1819"
#define CHARACTERISTIC_UUID_CHANGE_PASS "688091db-1736-4179-b7ce-e42a724a6a68"

//Define RFID PIN.
#define SS_PIN 5
#define RST_PIN 4

//There are 3 cards, The WHITE_CARD and BLUE_CARD_1 are the correct cards. Another card (non-define) are incorrect cards.
//You can change by define or non-define
#define WHITE_CARD
#define BLUE_CARD_1
//#define BLUE_CARD_2

//NUID cards
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

class MyCallbacks : public BLECharacteristicCallbacks
{
  void onRead(BLECharacteristic *pCharacteristic)
  {
    char txString[8];                 // make sure this is big enuffz
    dtostrf(txValue, 1, 1, txString); // float_val, min_width, digits_after_decimal, char_buffer

    pCharacteristic->setValue(txString);
  }

  void onWrite(BLECharacteristic *pCharacteristic)
  {
    BLEUUID uuid = pCharacteristic->getUUID();
    std::string rxValue = pCharacteristic->getValue();

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

    if (uuid.toString() == CHARACTERISTIC_UUID_CHANGE_PASS)
    {
      char char_array[rxValue.length()];
      for (int i = 0; i < rxValue.length(); i++)
      {
        Serial.print(rxValue[i]);
        char_array[i] = rxValue[i];
      }
      char_array[rxValue.length()] = NULL;

      for (int i = 0; char_array[i] != NULL; i++)
      {
        pass_correct[i] = char_array[i];
        pass_correct[i + 1] = NULL;
      }
    }
    if (uuid.toString() == CHARACTERISTIC_UUID_PASS)
    {
      if (pass_correct == rxValue)
      {
        Serial.println("Pass correct, open door");
        txValue = 1.1;
        dtostrf(txValue, 1, 1, txString);
        pCharacteristic->setValue(txString); // Sending  message
        pCharacteristic->notify();           // Send the value to the app!
        digitalWrite(LED, HIGH);
      }
      else
      {
        Serial.println("Pass incorrect, cannot open door");
        txValue = 0.0;
        dtostrf(txValue, 1, 1, txString);
        pCharacteristic->setValue(txString); // Sending  message
        pCharacteristic->notify();           // Send the value to the app!
        digitalWrite(LED, LOW);
      }
    }
  }
};

void setup()
{
  Serial.begin(115200);
  Serial.println(F("Initialize System"));
  pinMode(LED, OUTPUT);
  
  //Init RFID with SPI
  SPI.begin();
  rfid.PCD_Init();
  //Show Firmware Version: Confirm RFID are connected to ESP32
  Serial.print(F("Reader :"));
  rfid.PCD_DumpVersionToSerial();
  
  // Create the BLE Device
  BLEDevice::init("ESP32"); // Give it a name

  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create a BLE Characteristic
  pCharacteristic = pService->createCharacteristic(
      CHARACTERISTIC_UUID_CHECK,
      BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);

  pCharacteristic->addDescriptor(new BLE2902());
  pCharacteristic->setCallbacks(new MyCallbacks());

  pCharacteristic = pService->createCharacteristic(
      CHARACTERISTIC_UUID_PASS,
      BLECharacteristic::PROPERTY_WRITE);

  pCharacteristic->setCallbacks(new MyCallbacks());

  pCharacteristic = pService->createCharacteristic(
      CHARACTERISTIC_UUID_VERIFY,
      BLECharacteristic::PROPERTY_WRITE);
  pCharacteristic->setCallbacks(new MyCallbacks());

  pCharacteristic = pService->createCharacteristic(
      CHARACTERISTIC_UUID_CHANGE_PASS,
      BLECharacteristic::PROPERTY_WRITE);
  pCharacteristic->setCallbacks(new MyCallbacks());

  // Start the service
  pService->start();

  // Start advertising
  pServer->getAdvertising()->start();
  Serial.println("Waiting a client connection to notify...");
}

void getPassToOPenDoor()
{
  Serial.println("=================================================================");
  Serial.print("Type Your Password:");
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
      Serial.print("\t\t\tWRONG PASSWORD: ");
      Serial.println(i);
      if (i < 3)
      {
        Serial.print("Type Your Password Again:");
      }
      else
      {
        Serial.println("=================================================================");
        Serial.println("\t\t\tWARNING!!!");
      }
    }
    else
    {
      digitalWrite(LED, HIGH);
      Serial.println("\t\t\tYOUR DOOR IS OPENED");
      break;
    }
  }
}

void changePass()
{
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
      Serial.print("\t\t\tWRONG PASSWORD: ");
      Serial.println(i);
      if (i < 3)
      {
        Serial.println("=================================================================");
        Serial.print("Type your old Password again: ");
      }
      else
      {
        Serial.println("=================================================================");
        Serial.println("\t\t\tWARNING!!!");
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
      Serial.println("\t\tCHANGE YOUR NEW PASSWORD SUCCESS");
      break;
    }
  }
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

void closeDoor()
{
  Serial.println("=================================================================");
  digitalWrite(LED, LOW);
  Serial.println("\t\t\tYOUR DOOR IS CLOSED");
}

void loop()
{
  //============================================UART=========================================//
  Serial.println("================================================================="); // another function
  Serial.println("\t\t1.Type password to open your door");
  Serial.println("\t\t2.Change your password");
  Serial.println("\t\t3.Close your door");
  Serial.print("\t\tEnter your choose:");
  while (Serial.available() == 0)
  {
  } // wait for data available
  cmd = Serial.readString();
  cmd.trim();
  Serial.println(cmd);
  if (cmd == "1")
  {
    getPassToOPenDoor();
  }
  else if (cmd == "2")
  {
    changePass();
  }
  else if (cmd == "3")
  {
    closeDoor();
  }
  //============================================UART=========================================//

  //============================================BLE=========================================//
  if (deviceConnected)
  {
    Serial.println("Connected device"); // clean
  }
  delay(1000);

  // disconnecting
  if (!deviceConnected && oldDeviceConnected)
  {
    Serial.println("Disconnecting ...");
    delay(500);                  // give the bluetooth stack the chance to get things ready
    pServer->startAdvertising(); // restart advertising
    Serial.println("Start advertising");
    oldDeviceConnected = deviceConnected;
  }

  // connecting
  if (deviceConnected && !oldDeviceConnected)
  {
    oldDeviceConnected = deviceConnected;
  }
  //============================================BLE=========================================//
  
  //============================================RFID=========================================//
  runRFID();
  //============================================RFID=========================================//
}