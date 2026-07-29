// PSEUDO BS SO I UNDERSTAND WTF I NEED TO DO
// Connect to MQTT while running
// Receive input from A0 pin 
// Send

#include <SPI.h>
#include <MQTT.h>
#include <WiFiNINA.h>
//#include "secrets.h"


const char ssid[] = "rastic";
const char pass[] = "botbotbot";

int wifiStatus = WL_IDLE_STATUS;

// MQTT CONFIGURATION //

const char* BROKER = "rasticvm.internal";
const int PORT = 1883;
const char* COMMAND_TOPIC = "racuum/racuum-ACE9/setpoint";
const char* TEST_TOPIC = "racuum/racuum-ACE9/test";
const char* CLIENT_ID = "beren_akpinar";

const int sensorPins[] = {A0, A1, A2, A3, A4};
const int numSensors = 5;
const float vRef = 3.3;
const int adcResolution = 1023; 

const int channelIDs[] = {0,1,2,3,4};



WiFiClient wifiClient;
MQTTClient client;

unsigned long msgCount = 0;
unsigned long lastPublish = 0;
const unsigned long PUBLISH_INTERVAL = 3000; // 3 seconds

// connect to WiFi //

void connectToWIFI() 
{
  while (wifiStatus != WL_CONNECTED) 
  {
    Serial.print("Attempting to connect to: ");
    Serial.print(ssid);
    wifiStatus = WiFi.begin(ssid,pass);
  }
 
  Serial.println("\nconnected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

}

void connectToMQTT()
{
  client.begin(BROKER,PORT,wifiClient);

  while(!client.connect(CLIENT_ID))
  {
    Serial.println("\nConnecting to MQTT broker...");
    delay(1000);
  }

  Serial.println("\nConnected to MQTT Broker");
}


void configurePayload(char* payload, size_t payloadSize, int pin, int state){
  int channelID;
  int pressureCapacity;

  switch (pin) {
    case A0:
      channelID = 0;
      pressureCapacity = state * 10;   // A0-specific scaling
      break;
    case A1:
      channelID = 1;
      pressureCapacity = state * 25;   // A1 needs a bigger jump
      break;
    case A2:
      channelID = 2;
      pressureCapacity = state * 5;    // A2 more conservative
      break;
    case A3:
      channelID = 3;
      pressureCapacity = state * 50;
      break;
    case A4:
      channelID = 4;
      pressureCapacity = state * 15;
      break;
    case A5:
      channelID = 5;
      pressureCapacity = state * 30;
      break;
    default: // fallback case
      snprintf(payload, payloadSize, "{}");
      return; // exit early, nothing more to build
  }

  snprintf(payload, payloadSize, "{\"channels\":{\"%d\":%d}}", channelID, pressureCapacity);
}

void setChannelValues(int pin, int state){
  if (!client.connected()){
    connectToMQTT();
  }

  char payload[64];
  configurePayload(payload,sizeof(payload), pin, state);

  client.publish(COMMAND_TOPIC, payload, false, 1);

  Serial.println("Published payload: ");
  Serial.println(payload);

}

// Setup for input
//const int sensorPin = A0;

void setup()
  {
    Serial.begin(9600);

    while(!Serial)
    {
      connectToWIFI();
      connectToMQTT();
    }
  }

void loop() {
  client.loop();

  if (!client.connected()) {
    connectToWIFI();
    connectToMQTT();
  }

  unsigned long now = millis();
  if (now - lastPublish >= PUBLISH_INTERVAL) {
    lastPublish = now;
    
    // for every sensor in the list of sensors


    for (int i = 0; i < numSensors; i++) {
      int rawValue = analogRead(sensorPins[i]); // read raw value from the pin
      float voltage = (rawValue / (float)adcResolution) * vRef; // convert to volts
      int state = (voltage < 2.0) ? 1 : 0; // ON if below 2.0, else OFF

      if (sensorPins[i] == A0) {
        Serial.println("A0 triggered");
        setChannelValues(sensorPins[i], state);
      }
      else if (sensorPins[i] == A1) {
        Serial.println("A1 triggered");
        setChannelValues(sensorPins[i], state);
      }
      else if (sensorPins[i] == A2) {
        Serial.println("A2 triggered");
        setChannelValues(sensorPins[i], state);
      }
      else if (sensorPins[i] == A3) {
        Serial.println("A3 triggered");
        setChannelValues(sensorPins[i], state);
      }
      // add more else if blocks for additional pins as needed

      delay(500);
    }
  }

  // Read input from A0
  // int rawValue = analogRead(sensorPin); // read raw val from pin
  // float VOLTAGE = (rawValue / (float)adcResolution) * vRef; // convert to volts

  

  // Baseline voltage = ~ 2.25
  // Send message to setpoint if V < 2
  // if loop here
  // must tag PIN
  // if voltage is below 2, send message to setpoint (casuing state one for one corresponing pin )
  // bend by 50 whatever through we would send the payload information 
  // else go back to neutral state via setpoint payload if voltage above 2 

  
}

