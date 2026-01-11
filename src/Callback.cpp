#include "Callback.h"
#include "Updater.h"
#include "Config.h"

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  String msg;
  for (uint16_t i = 0; i < length; i++) msg += (char)payload[i];

  if (msg.indexOf("Update") >= 0) updaterStart();
  if (msg.indexOf("Disable") >= 0) alert = false;
  if (msg.indexOf("Enable")  >= 0) alert = true;
  if (msg.indexOf("Reboot")  >= 0) ESP.restart();
}
