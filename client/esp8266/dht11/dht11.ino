#include <DHT.h>
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ArduinoNATS.h>
#include <ArduinoJson.h>
#include "secrets.h"

#define DHTPIN D4
#define DHTTYPE DHT11
#define DHTVPIN D8
#define NODE_ID 3
const int sleepSeconds = 900; // 15 minutes

/*
 * Physical Connections:
 * Board: Wemos D1 mini r2
 * DHT11:
 *  VCC -> D8
 *  GND -> GND
 *  OUT -> D4
 *
 * Wemos D1 mini:
 * D0 -> RST
 *
 * Power supply:
 * Power source + -> 5v
 * Power source - -> GND
*/
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

void connect_wifi() {
    Serial.println("Connecting to wifi");
	WiFi.mode(WIFI_STA);
	WiFi.begin(ssid, password);
    Serial.println();
    
	while (WiFi.status() != WL_CONNECTED) {
        int wifi_tries=0; 
        Serial.print(".");
		if(wifi_tries == 100) {
            deep_sleep();
            return;
        }
        delay(10);
        Serial.print(".");
	}
  Serial.println("Connected to wifi");
}

void nats_on_connect() {
    SensorData sData = readSensorData();
	StaticJsonDocument<100> jsonDocument;
	jsonDocument["id"] = NODE_ID;
	JsonObject sensors = jsonDocument.createNestedObject("sensors");
	sensors["humidity"] = sData.humidity;
	sensors["temperature"] = sData.temperature;
	char jsonBuffer[100];
	serializeJson(jsonDocument, jsonBuffer);
	nats.publish(nats_subject, jsonBuffer);
}

void setup() {
  Serial.begin(115200);
  pinMode(D0, WAKEUP_PULLUP);
  pinMode(DHTVPIN, OUTPUT);
  digitalWrite(DHTVPIN, LOW);
  connect_wifi();
  nats.on_connect = nats_on_connect;
  nats.connect();
  publish_nats_message();
  deep_sleep();
}

void loop() {
	// not used due to deep sleep
}

void publish_nats_message(){
  for(int i=0; i<5; i++){
    Serial.println(i);
    if (WiFi.status() != WL_CONNECTED) connect_wifi();
	  nats.process();
    delay(100);
  }
}

SensorData readSensorData(){
  dht.begin();
  digitalWrite(DHTVPIN, HIGH);
  delay(1000);
  SensorData sensorData;
  sensorData.humidity = -300;
  sensorData.temperature = -300;
  sensorData.dhtError = 0;
  for (int i=0; i<50; i++) {
    //digitalWrite(DHTVPIN, HIGH);
    if ((sensorData.temperature == -300 && sensorData.humidity == -300) || 
      (isnan(sensorData.humidity) || isnan(sensorData.temperature))) {
      Serial.println("Reading values");
      sensorData.humidity = dht.readHumidity();
      sensorData.temperature = dht.readTemperature();
      if (isnan(sensorData.humidity) || isnan(sensorData.temperature)) {
        Serial.println("Failed to read from DHT sensor!");
        sensorData.dhtError = 1;
      } else {
        sensorData.dhtError = 0;
        break;
      }
    }
    delay(500);
  }
  digitalWrite(DHTVPIN, LOW);
  if (sensorData.dhtError == 1) {
    deep_sleep();
  }
  return sensorData;
}

void deep_sleep() {
  ESP.deepSleep(sleepSeconds * 1000000);
}
