#include <WiFi.h>  
#include <PubSubClient.h>
#include "DHT.h"

// DHT Sensor settings
#define DHTTYPE DHT11
#define DHTPin 4  // Using GPIO 4 now

DHT dht(DHTPin, DHTTYPE);

// WiFi credentials
const char* ssid = "A";             // Your WiFi SSID
const char* password = "12345678";   // Your WiFi password

// MQTT broker settings
//const char* mqtt_server = "broker.hivemq.com";
const char* mqtt_server = "test.mosquitto.org";
const char* MQTT_username = NULL; 
const char* MQTT_password = NULL;

// Initialize WiFi and MQTT clients
WiFiClient espClient;
PubSubClient client(espClient);

unsigned long previousMillis = 0; // For timing MQTT publish

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println();
  Serial.print("WiFi connected - ESP IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  // Loop until reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP32Client-" + String(random(0xffff), HEX);
    if (client.connect(clientId.c_str(), MQTT_username, MQTT_password)) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  dht.begin();
  setup_wifi();
  client.setServer(mqtt_server, 1883);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop(); // Keep MQTT connection alive

  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis > 10000) { // Every 10 seconds
    previousMillis = currentMillis;

    float humidity = dht.readHumidity();
    float temperatureC = dht.readTemperature();
    float temperatureF = dht.readTemperature(true);

    if (isnan(humidity) || isnan(temperatureC) || isnan(temperatureF)) {
      Serial.println("Failed to read from DHT sensor!");
      return;
    }

    // Publish readings to MQTT topics
    client.publish("iotfrontier/temperature", String(temperatureC).c_str());
    client.publish("iotfrontier/humidity", String(humidity).c_str());

    // Print readings to Serial Monitor
    Serial.print("Humidity: ");
    Serial.print(humidity);
    Serial.println(" %");
    Serial.print("Temperature: ");
    Serial.print(temperatureC);
    Serial.println(" ºC");
    Serial.print(temperatureF);
    Serial.println(" ºF");
  }
}
