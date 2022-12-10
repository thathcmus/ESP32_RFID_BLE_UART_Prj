#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

BLEServer *pServer = NULL;
BLECharacteristic *pCharacteristic;
bool deviceConnected = false;
bool oldDeviceConnected = false;
float txValue = 0.0; // 0.0 is incorrect, 1.1 is correct
const int LED = 2; // Could be different depending on the dev board. I used the DOIT ESP32 dev board.
// Password for the door
char pass_correct[] = "Tung123";
char txString[8];


#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E" // UART service UUID
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};

class MyCallbacks: public BLECharacteristicCallbacks {
  void onRead(BLECharacteristic *pCharacteristic) 
  {
    char txString[8]; // make sure this is big enuffz
    dtostrf(txValue, 1, 1, txString); // float_val, min_width, digits_after_decimal, char_buffer
    
    pCharacteristic->setValue(txString);
  }
  
  void onWrite(BLECharacteristic *pCharacteristic) 
  {
    std::string rxValue = pCharacteristic->getValue();

    if (rxValue.length() > 0)
    {
      Serial.println("*********");
      Serial.print("Received Value: ");

      for (int i = 0; i < rxValue.length(); i++)
      {
        Serial.print(rxValue[i]);
      }
      Serial.println();
      if(pass_correct == rxValue)
      {
        Serial.println("Pass correct, open door");
        txValue = 1.1;
        dtostrf(txValue, 1, 1, txString); 
        pCharacteristic->setValue(txString); // Sending  message   
        pCharacteristic->notify(); // Send the value to the app!    
        digitalWrite(LED, HIGH);
      }
      else
      {
        Serial.println("Pass incorrect, cannot open door");
        txValue = 0.0;
        dtostrf(txValue, 1, 1, txString); 
        pCharacteristic->setValue(txString); // Sending  message     
        pCharacteristic->notify(); // Send the value to the app!
        digitalWrite(LED, LOW);
      }
      // Do stuff based on the command received from the app
      // if (rxValue.find("A") != -1) 
      // { 
      //   Serial.println("Turning ON!");
      //   digitalWrite(LED, HIGH);
      // }
      // else if (rxValue.find("B") != -1)
      // {
      //   Serial.println("Turning OFF!");
      //   digitalWrite(LED, LOW);
      // }
      Serial.println();
      Serial.println("*********");
    }
  }
};

void setup() {
  Serial.begin(115200);

  pinMode(LED, OUTPUT);

  // Create the BLE Device
  BLEDevice::init("ESP32 open door"); // Give it a name

  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());


  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create a BLE Characteristic
  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID_TX,
                      BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY
                    );
                      
  pCharacteristic->addDescriptor(new BLE2902());
  pCharacteristic->setCallbacks(new MyCallbacks());

  BLECharacteristic *pCharacteristic = pService->createCharacteristic(
                                         CHARACTERISTIC_UUID_RX,
                                         BLECharacteristic::PROPERTY_WRITE
                                       );

  pCharacteristic->setCallbacks(new MyCallbacks());

  // Start the service
  pService->start();

  // Start advertising
  pServer->getAdvertising()->start();
  Serial.println("Waiting a client connection to notify...");
}

void loop() {

  if (deviceConnected) 
  {
    Serial.println("Connected device");
  }
  delay(1000);
    

  // disconnecting
  if (!deviceConnected && oldDeviceConnected) 
  {
    Serial.println("Disconnecting ...");
    delay(500); // give the bluetooth stack the chance to get things ready
    pServer->startAdvertising(); // restart advertising
    Serial.println("Start advertising");
    oldDeviceConnected = deviceConnected;
  }

  // connecting
  if (deviceConnected && !oldDeviceConnected) 
  {
    oldDeviceConnected = deviceConnected;
  }

}