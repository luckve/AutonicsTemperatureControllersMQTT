#ifndef CALLBACK_H
#define CALLBACK_H

#include <Arduino.h>
void mqttCallback(char* topic, byte* payload, unsigned int length);

#endif
