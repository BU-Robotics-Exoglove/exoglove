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
const char* CLIENT_ID = "berenakpinar";


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

// void messageReceived(String &topic, String &payload) {
//   Serial.println("incoming: " + topic + " - " + payload);

//   // Note: Do not use the client in the callback to publish, subscribe or
//   // unsubscribe as it may cause deadlocks when other things arrive while
//   // sending and receiving acknowledgments. Instead, change a global variable,
//   // or push to a queue and handle it in the loop after calling `client.loop()`.
// }

void setup() 
{
  Serial.begin(9600);

  while (!Serial)
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
             "{\"test\":\"ping\",\"count\":%lu,\"time\":%lu}",
             msgCount, now);

    bool result = client.publish(TEST_TOPIC, payload, false, 1); // qos=1, matches Python

    if (result) {
      Serial.print("Sent test message #");
      Serial.print(msgCount);
      Serial.print(" to ");
      Serial.println(TEST_TOPIC);
    } else {
      Serial.print("Failed to send test message #");
      Serial.println(msgCount);
    }
    msgCount++;

    // ---- Original payload publish section (commented out) ----
    // setChannelValues("1", 95);
    // delay(3000);
    // setChannelValues("1", 0);
    // delay(3000);
  }
}