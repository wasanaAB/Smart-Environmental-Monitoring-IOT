#include <WiFi.h>
#include <PubSubClient.h>

// Pin definitions for RGB LED
#define redPin   14
#define greenPin 12
#define bluePin  27

// Pin for water level sensor
#define LedPin 4        
#define sensorPower 26
#define sensorPin 36

// WiFi credentials
const char* ssid = "A";
const char* password = "12345678";

// MQTT broker settings
const char* mqtt_server = "test.mosquitto.org";  // or your own local broker
const char* mqtt_client_id = "ESP32Client001";
const char* mqtt_username = "";
const char* mqtt_password = "";

// MQTT topics
const char* mqtt_topic_water = "iotfrontier/waterlevel";
const char* mqtt_topic_led = "iotfrontier/ledControl";
const char* mqtt_topic_color = "iotfrontier/color";

WiFiClient espClient;
PubSubClient client(espClient);

unsigned long previousMillis = 0;

// This functions connects your ESP32 to your router
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
  Serial.println("");
  Serial.print("WiFi connected - ESP IP address: ");
  Serial.println(WiFi.localIP());
}

void setColor(int R, int G, int B) {
  analogWrite(redPin,   R);
  analogWrite(greenPin, G);
  analogWrite(bluePin,  B);
}

// This function is executed when some device publishes a message to a topic that your ESP32 is subscribed to.
void callback(String topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    messageTemp += (char)message[i];
  }
  Serial.println(messageTemp);
  
  // Parse the message to extract RGB values from "rgb(r, g, b)" format
  if (topic == mqtt_topic_color) {
    int r, g, b;
    
    // Find the start and end of the RGB values
    int startIndex = messageTemp.indexOf('(') + 1;
    int endIndex = messageTemp.indexOf(')');
    
    // Extract the substring between the parentheses
    String rgbValues = messageTemp.substring(startIndex, endIndex);
    
    // Split the values by comma
    int comma1 = rgbValues.indexOf(',');
    int comma2 = rgbValues.indexOf(',', comma1 + 1);
    
    r = rgbValues.substring(0, comma1).toInt();
    g = rgbValues.substring(comma1 + 1, comma2).toInt();
    b = rgbValues.substring(comma2 + 1).toInt();
    
    // Set the LED color
    setColor(r, g, b);
  }
}

// Reconnect to MQTT broker if disconnected
void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect(mqtt_client_id, mqtt_username, mqtt_password)) {
      Serial.println("connected");
      client.subscribe(mqtt_topic_color);  // Subscribe to the color change topic
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  pinMode(redPin,   OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin,  OUTPUT);
  pinMode(LedPin,   OUTPUT);
  pinMode(sensorPower, OUTPUT);
  pinMode(sensorPin, INPUT);

  Serial.begin(115200);
  setup_wifi();

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  
  // Water level sensing and publishing
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis > 30000) {  // Every 30 seconds
    previousMillis = currentMillis;
    
    int waterLevel = analogRead(sensorPin);  // Read the water level
    Serial.print("Water Level: ");
    Serial.println(waterLevel);
    
    // Publish the water level to the MQTT topic
    String waterLevelStr = String(waterLevel);
    client.publish(mqtt_topic_water, waterLevelStr.c_str());
  }
}
