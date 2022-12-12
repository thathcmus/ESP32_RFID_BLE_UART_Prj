/* Includes ----------------------------------------------------------- */
#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

/* Private defines ---------------------------------------------------- */
#define SERVICE_UUID                        "844fc6d6-1c7e-461a-abb9-3a9a9fdd597e" 
#define CHARACTERISTIC_UUID_PASS            "909aa990-1e33-4bf6-bfbd-11efd5b571a4"
#define CHARACTERISTIC_UUID_CHECK           "80a80bef-ea4c-426f-885a-5c0a34757a54"
#define CHARACTERISTIC_UUID_VERIFY          "0515e27d-dd91-4f96-9452-5f43649c1819"
#define CHARACTERISTIC_UUID_CHANGE_PASS     "688091db-1736-4179-b7ce-e42a724a6a68"
#define LED  2

/* Private macros ----------------------------------------------------- */

/* Public variables --------------------------------------------------- */
BLEServer *pServer = NULL;
BLECharacteristic *pCharacteristic;
bool deviceConnected = false;
bool oldDeviceConnected = false;
// 0.0 is incorrect, 1.1 is correct
float txValue = 0.0; 

// Password for the door
char pass_correct[] = "Iot123";
char passwordTemp[] = "";
char txString[8];
String cmd;

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

/**
 * @brief Type your new password to change password
 */
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

/**
 * @brief Close the door
 */
void closeDoor()
{
  Serial.println("=================================================================");
  digitalWrite(LED, LOW);
  Serial.println("\t\t\tYOUR DOOR IS CLOSED");
}

/* Set up ------------------------------------------------------------- */
void setup()
{
  Serial.begin(115200);

  pinMode(LED, OUTPUT);

  // Create the BLE Device
  BLEDevice::init("ESP32"); // Give it a name

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
  Serial.println("Waiting a client connection to notify...");
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
  Serial.print("\t\tEnter your choose:");
}

/* Main loop ---------------------------------------------------------- */
void loop()
{
  //============================================UART=========================================//
  dashboard(); // Display feature to choose
  while (Serial.available() == 0)
  {
    // wait for data available
  } 
  cmd = Serial.readString();
  cmd.trim();
  Serial.println(cmd);
  
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
  //============================================UART=========================================//

  //============================================BLE=========================================//
  // BLE connected
  if (deviceConnected)
  {
    // TO DO
  }
  delay(1000);

  // BLE disconnecting
  if (!deviceConnected && oldDeviceConnected)
  {
    // restart advertising
    pServer->startAdvertising(); 
    oldDeviceConnected = deviceConnected;
  }

  // BLE connecting
  if (deviceConnected && !oldDeviceConnected)
  {
    oldDeviceConnected = deviceConnected;
  }
  //============================================BLE=========================================//
}
/* End of file -------------------------------------------------------- */