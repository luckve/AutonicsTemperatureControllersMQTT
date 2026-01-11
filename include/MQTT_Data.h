#ifndef MQTT_DATA_H
#define MQTT_DATA_H

#include <Arduino.h>

void mqttSetup();
void mqttLoop();
void mqttPublishStatus(const char* type);

#endif
