#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// Music Libraries

#include <melody_player.h>
#include <melody_factory.h>

int buzzerPin = 13;

// Update these with values suitable for your network.
const char* ssid = "ThisIsntAWifiSignal";
const char* password = "5C68ED27D3";
const char* mqtt_server = "broker.emqx.io";

WiFiClient espClient;
PubSubClient client(espClient);
MelodyPlayer player(buzzerPin, HIGH);
StaticJsonDocument<200> doc;

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());
}

void callback(char* topic, byte* payload, unsigned int length) {

  char arr[length];
  
  for (int i = 0; i < length; i++) {
    arr[i] = (char)payload[i];
  }

  String json = String(arr);
  
  // Deserialize the JSON document
  DeserializationError error = deserializeJson(doc, json);
  
  // Test if parsing succeeds.
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    return;
  }

  String note = doc["note"];
  Serial.println(note);
  
  String notes[] = { note };

  Melody melody = MelodyFactory.load("Note", 175, notes, 1);

  player.playAsync(melody);
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // ... and resubscribe
      client.subscribe("IDKWhatImDoingHere");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
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
}
