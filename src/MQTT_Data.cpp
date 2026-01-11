#include "MQTT_Data.h"
#include "Config.h"
#include "Callback.h"
#include <WiFi.h>
#include <PubSubClient.h>

WiFiClient wifiClient;
PubSubClient client(wifiClient);

const char* user_mqtt = "tt_mqtt";
const char* pass_mqtt = "12345678";
const char* firmware  = "0.16";
const char* IP_ID     = "DEV-IOT_SVR-001";

static unsigned long lastReconnect = 0;
static String node_id;

void mqttSetup() {
  node_id = String(IP_ID) + String(random(0xffff), HEX);
  client.setServer(IPAddress(192,168,10,77), 1883);
  client.setCallback(mqttCallback);
}

void mqttLoop() {
  if (!client.connected() && millis() - lastReconnect > 5000) {
    lastReconnect = millis();
    client.connect(node_id.c_str(), user_mqtt, pass_mqtt);
    client.subscribe(IP_ID);
  }
  client.loop();
}

void mqttPublishStatus(const char* type) {
  if (!client.connected()) return;

  String payload = "{";
  payload += "\"type\":\"" + String(type) + "\",";
  payload += "\"PV1\":\"" + String(pv1) + "\",";
  payload += "\"SV1\":\"" + String(sv1) + "\"}";
  client.publish("IOT/SERVER/TEMP1", payload.c_str());
}
