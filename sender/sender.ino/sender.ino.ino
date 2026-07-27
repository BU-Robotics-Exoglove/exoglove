// PSEUDO BS SO I UNDERSTAND WTF I NEED TO DO
// Connect to MQTT while running
// Receive input from A0 pin 
// Send

#include <SPI.h>
#include <MQTT.h>
#include <WiFiNINA.h>
#include "arduino_secrets.h"

char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;

const char ssid[] = ssid
const char pass[] = pass;
int wifiStatus = WL_IDLE_STATUS;

// MQTT CONFIGURATION //

const char* BROKER = "rasticvm.internal";
const int PORT = 1883;
const char* COMMAND_TOPIC = "racuum/racuum-ACE9/setpoint";
const char* TEST_TOPIC = "racuum/racuum-ACE9/test";
const char* CLIENT_ID = "nico_computar";


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

void setChannelValues(String channelID, int pressureCapacity){
  if (!client.connected()){
    connectToMQTT();
  }

  char payload[64];
  snprintf(payload, sizeof(payload), "{\"channels\":{\"%d\":%d}}", channelID, pressureCapacity);

  client.publish(COMMAND_TOPIC, payload, false, 1);

  Serial.println("Published payload: ");
  Serial.println(payload);

}

// Setup for input
const int sensorPin = A0;
const float vRef = 3.3;
const int adcResolution = 1023;

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

    char payload[96];
    snprintf(payload, sizeof(payload),
    "{setChannelValues}")
  }

  // Read input from A0
  int rawValue = analogRead(sensorPin); // read raw val from pin
  float voltage = (rawValue / (float)adcResolution) * vRef; // convert to volts

  // Baseline voltage = ~ 2.25
  // Send message to setpoint if V < 2
  // if loop here
}

