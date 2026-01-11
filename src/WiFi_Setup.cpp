#include "WiFi_Setup.h"
#include "Config.h"
#include <WiFi.h>

const char* ssid = "TT-IOT";
const char* password = "ttiot2019";

static unsigned long lastCheck = 0;

void wifiSetup() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
}

void wifiLoop() {
  if (millis() - lastCheck > 3000) {
    lastCheck = millis();
    if (WiFi.status() != WL_CONNECTED) {
      WiFi.reconnect();
    }
  }
}
