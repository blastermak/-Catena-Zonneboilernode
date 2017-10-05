#include <OneWire.h>
#include <DallasTemperature.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <readNodeData.h>
 
// Data wire is plugged into pin 2 on the Arduino
#define ONE_WIRE_BUS 14
 
// Setup a oneWire instance to communicate with any OneWire devices 
// (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);
 
// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);

uint8_t zonneBoilerOnder[8]   = {0x28, 0x35, 0xD3, 0x0, 0x0, 0x0, 0x80, 0x84};
uint8_t zonneBoilerMidden[8]  = {0x28, 0xFF, 0x2, 0x45, 0x84, 0x16, 0x4, 0x71};
uint8_t zonneBoilerBoven[8]   = {0x28, 0xFF, 0x73, 0x45, 0x84, 0x16, 0x4, 0xD1};

const char* ssid = "Catena - kader";
const char* password = "dezekeyisgeheim";
const char* mqtt_server = "jimmak.nl";

long lastReconnectAttempt = 0;

unsigned long vorig = 0;
const long interval = 300000;

readNodeData sensoren[3] = {readNodeData("zonneboiler", "onder",  "temp", zonneBoilerOnder, sensors),
                            readNodeData("zonneboiler", "midden", "temp", zonneBoilerMidden, sensors),
                            readNodeData("zonneboiler", "boven",  "temp", zonneBoilerBoven, sensors)};

WiFiClient espClient;
PubSubClient client(espClient);

boolean reconnect() {
  if (client.connect("Catena-node-zonneboiler")) {
    // Once connected, publish an announcement...
  }
  return client.connected();
}

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}
 
void setup(void){
  // start serial port
  Serial.begin(9600);
  Serial.println("Catena - zonneboiler - node - v0.2");

  // Start up the library
  sensors.begin();

  setup_wifi();
  client.setServer(mqtt_server, 1883);
//  client.setCallback(callback);
  lastReconnectAttempt = 0;
}


 
void loop(void) {
  if (!client.connected()) {
    long now = millis();
    if (now - lastReconnectAttempt > 5000) {
      lastReconnectAttempt = now;
      // Attempt to reconnect
      if (reconnect()) {
        lastReconnectAttempt = 0;
      }
    }
  } else {
    long nu = millis();
    if (nu - vorig >= interval){
      vorig = nu;
      for (int i = 0; i < 3; i++){
        char msg[10];
        dtostrf(sensoren[i].returnTemp(), 5, 2, msg);
        client.publish(sensoren[i].prepareMqttChannel(),msg);
        Serial.println(msg);
      }

    }
  }
}
