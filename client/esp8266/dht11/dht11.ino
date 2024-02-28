#include <DHT.h>
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <ArduinoNATS.h>
#include "secrets.h"  // Make sure you define nats_address, NATS_DEFAULT_PORT, ssid, and password in this file

#define DHTPIN D4
#define DHTTYPE DHT11
#define DHTVPIN D8
#define NODE_ID 3
const int sleepSeconds = 900; // 15 minutes

// Define a struct to hold sensor data
struct SensorData {
  float humidity;
  float temperature;
  int dhtError;
};

DHT dht(DHTPIN, DHTTYPE);
WiFiClient client;
NATS nats(
  &client,
  nats_address, NATS_DEFAULT_PORT
);

void setup() {
  Serial.begin(115200);
  delay(10);
  pinMode(D0, WAKEUP_PULLUP);
  pinMode(DHTVPIN, OUTPUT);
  digitalWrite(DHTVPIN, LOW);
  dht.begin();
  delay(2000);

  WiFi.begin(ssid, password);
  
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  int wifi_tries=0; 
  while (WiFi.status() != WL_CONNECTED) {
    wifi_tries++;
    if(wifi_tries == 100) {
      deep_sleep();
      return;
    }
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  
  digitalWrite(DHTVPIN, HIGH);
  SensorData sensorData;
  sensorData.humidity = -300;
  sensorData.temperature = -300;
  sensorData.dhtError = 0;
  for (int i=0; i<3; i++) {
    if ((sensorData.temperature == -300 && sensorData.humidity == -300) || 
      (isnan(sensorData.humidity) || isnan(sensorData.temperature))) {
      Serial.println("Reading values");
      sensorData.humidity = dht.readHumidity();
      sensorData.temperature = dht.readTemperature();
      if (isnan(sensorData.humidity) || isnan(sensorData.temperature)) {
        Serial.println("Failed to read from DHT sensor!");
        sensorData.dhtError = 1;
      } else {
        break;
      }
    }
  }
  digitalWrite(DHTVPIN, LOW);
  
  if (sensorData.dhtError == 0) {
    // Convert sensorData to JSON
    char jsonBuffer[256];
    size_t jsonSize = serializeSensorDataToJson(sensorData, jsonBuffer, sizeof(jsonBuffer));

    // Convert jsonSize to string representation
    char jsonSizeStr[32];
    snprintf(jsonSizeStr, sizeof(jsonSizeStr), "%u", jsonSize);

    // Publish the JSON message to NATS.io broker
    nats.publish(nats_subject, jsonBuffer, jsonSizeStr);
  }
  
  Serial.println("Waiting 20 secs");
  deep_sleep();
}

void loop() {
  // loop not used due to deep sleep
}

void deep_sleep() {
  ESP.deepSleep(sleepSeconds * 1000000); // sleepSeconds needs to be defined
}

size_t serializeSensorDataToJson(const SensorData &data, char *buffer, size_t bufferSize) {
  // Create a JSON object
  StaticJsonDocument<200> doc;
  doc["id"] = NODE_ID;
  JsonObject sensors = doc.createNestedObject("sensors");
  sensors["humidity"] = data.humidity;
  sensors["temperature"] = data.temperature;

  // Serialize the JSON object to the provided buffer
  return serializeJson(doc, buffer, bufferSize);
}
