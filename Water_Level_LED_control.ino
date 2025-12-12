#include <WiFi.h>
#include <PubSubClient.h>

// Pin definitions
#define LedPin 4        // ESP32 built-in LED
#define sensorPower 26
#define sensorPin 36    // ADC1 channel 0

// WiFi credentials
const char* ssid = "A";
const char* password = "12345678";

// MQTT broker settings (change IP or domain as needed)
const char* mqtt_server = "test.mosquitto.org";  

// MQTT credentials (leave empty if not required)
const char* mqtt_client_id = "ESP32Client001";
const char* mqtt_username = "";
const char* mqtt_password = "";

// MQTT topics
const char* mqtt_topic_water = "iotfrontier/waterlevel";
const char* mqtt_topic_led = "iotfrontier/ledControl";

// Clients
WiFiClient espClient;
PubSubClient client(espClient);

int waterLevel = 0;

void setup_wifi() {
    delay(10);
    Serial.println();
    Serial.print("Connecting to WiFi: ");
    Serial.println(ssid);

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("\nWiFi connected!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
    String msg;
    for (unsigned int i = 0; i < length; i++) {
        msg += (char)payload[i];
    }

    Serial.print("Message received [");
    Serial.print(topic);
    Serial.print("]: ");
    Serial.println(msg);

    if (msg == "1") {
        digitalWrite(LedPin, HIGH);
        Serial.println("LED ON");
    } else if (msg == "2") {
        digitalWrite(LedPin, LOW);
        Serial.println("LED OFF");
    }
}

void reconnect() {
    while (!client.connected()) {
        Serial.print("Attempting MQTT connection...");
        if (client.connect(mqtt_client_id, mqtt_username, mqtt_password)) {
            Serial.println("connected!");
            client.subscribe(mqtt_topic_led);
        } else {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" retrying in 5 seconds...");
            delay(5000);
        }
    }
}

void setup() {
    Serial.begin(115200);

    pinMode(LedPin, OUTPUT);
    pinMode(sensorPower, OUTPUT);
    digitalWrite(sensorPower, LOW);

    setup_wifi();
    client.setServer(mqtt_server, 1883);
    client.setCallback(callback);
}

void loop() {
    if (!client.connected()) {
        reconnect();
    }
    client.loop();

    // Read analog value from water level sensor
    digitalWrite(sensorPower, HIGH);
    delay(10); // wait for sensor to stabilize
    waterLevel = analogRead(sensorPin);  // value: 0 - 4095 (for 0 - 3.3V)
    delay(10);
    digitalWrite(sensorPower, LOW);

    Serial.print("Water level: ");
    Serial.println(waterLevel);

    // Publish to MQTT
    String payload = String(waterLevel);
    client.publish(mqtt_topic_water, payload.c_str());

    delay(1000);
}
