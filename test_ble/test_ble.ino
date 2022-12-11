#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

BLEServer *pServer = NULL;
BLECharacteristic *pCharacteristic;

bool deviceConnected = false;
bool oldDeviceConnected = false;
float txValue = 0.0;  // 0.0 is incorrect, 1.1 is correct
const int LED = 2;    // Could be different depending on the dev board. I used the DOIT ESP32 dev board.
// Password for the door
char pass_correct[] = "Tung123";
// String pass_correct = String("Tung123");
char verify[] = "verify";
char change_pass[] = "change_pass";
char txString[8];


#define SERVICE_UUID "844fc6d6-1c7e-461a-abb9-3a9a9fdd597e"  // UART service UUID
#define CHARACTERISTIC_UUID_PASS "909aa990-1e33-4bf6-bfbd-11efd5b571a4"
#define CHARACTERISTIC_UUID_CHECK "80a80bef-ea4c-426f-885a-5c0a34757a54"
#define CHARACTERISTIC_UUID_VERIFY "0515e27d-dd91-4f96-9452-5f43649c1819"
#define CHARACTERISTIC_UUID_CHANGE_PASS "688091db-1736-4179-b7ce-e42a724a6a68"



class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer *pServer) {
    deviceConnected = true;
  };

  void onDisconnect(BLEServer *pServer) {
    deviceConnected = false;
  }
};

class MyCallbacks : public BLECharacteristicCallbacks {
  void onRead(BLECharacteristic *pCharacteristic) {
    char txString[8];                  // make sure this is big enuffz
    dtostrf(txValue, 1, 1, txString);  // float_val, min_width, digits_after_decimal, char_buffer

    pCharacteristic->setValue(txString);
  }

  void onWrite(BLECharacteristic *pCharacteristic) {
    BLEUUID uuid = pCharacteristic->getUUID();
    std::string rxValue = pCharacteristic->getValue();

    if (uuid.toString() == CHARACTERISTIC_UUID_VERIFY) {
      if (pass_correct == rxValue) {
        txValue = 1.1;
        dtostrf(txValue, 1, 1, txString);
        pCharacteristic->setValue(txString);  // Sending  message
        pCharacteristic->notify();            // Send the value to the app!
      }
    }
  
    if(uuid.toString() == CHARACTERISTIC_UUID_CHANGE_PASS)
    {
      char char_array[rxValue.length()];
      for (int i = 0; i < rxValue.length(); i++) {
          Serial.print(rxValue[i]);
          char_array[i] = rxValue[i];          
      }
      char_array[rxValue.length()] = NULL;
      Serial.println();
      Serial.println("Char: ");
      Serial.print(char_array);

       for (int i = 0; char_array[i] != NULL ; i++) {
            pass_correct[i] = char_array[i];
            pass_correct[i+1] = NULL;
      }
      // strncpy(pass_correct, char_array, 4);
      if(pass_correct == char_array)
      {
        Serial.println("Password is Tung");
      }
    }
    if (uuid.toString() == CHARACTERISTIC_UUID_PASS) {
      if (rxValue.length() > 0) {
        Serial.println("*********");
        Serial.print("Received Value: ");

        for (int i = 0; i < rxValue.length(); i++) {
          Serial.print(rxValue[i]);
        }
        Serial.println();

        if (pass_correct == rxValue) {
          Serial.println("Pass correct, open door");
          txValue = 1.1;
          dtostrf(txValue, 1, 1, txString);
          pCharacteristic->setValue(txString);  // Sending  message
          pCharacteristic->notify();            // Send the value to the app!
          digitalWrite(LED, HIGH);
        } else {
          Serial.println("Pass incorrect, cannot open door");
          txValue = 0.0;
          dtostrf(txValue, 1, 1, txString);
          pCharacteristic->setValue(txString);  // Sending  message
          pCharacteristic->notify();            // Send the value to the app!
          digitalWrite(LED, LOW);
        }
      }



      Serial.println();
      Serial.println("*********");
    }
  }
};

void setup() {
  Serial.begin(115200);

  pinMode(LED, OUTPUT);

  // Create the BLE Device
  BLEDevice::init("ESP32");  // Give it a name

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

void loop() {

  if (deviceConnected) {
    Serial.println("Connected device");
  }
  delay(1000);


  // disconnecting
  if (!deviceConnected && oldDeviceConnected) {
    Serial.println("Disconnecting ...");
    delay(500);                   // give the bluetooth stack the chance to get things ready
    pServer->startAdvertising();  // restart advertising
    Serial.println("Start advertising");
    oldDeviceConnected = deviceConnected;
  }

  // connecting
  if (deviceConnected && !oldDeviceConnected) {
    oldDeviceConnected = deviceConnected;
  }

  Serial.println("Pass now: ");
  // for (int i = 0; i < pass_correct.length(); i++) {
  //         Serial.print(pass_correct[i]);
  //     }
  Serial.println(pass_correct);

}