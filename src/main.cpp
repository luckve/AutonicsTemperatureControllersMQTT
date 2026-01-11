#include <Arduino.h>
#include "WiFi_Setup.h"
#include "MQTT_Data.h"
#include "Req_Modbus.h"
#include "Updater.h"

void setup() {
  Serial.begin(9600);
  wifiSetup();
  mqttSetup();
  modbusSetup();
}

void loop() {
  wifiLoop();
  mqttLoop();
  modbusLoop();
  updaterLoop();
}
