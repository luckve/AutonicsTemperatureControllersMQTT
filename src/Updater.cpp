#include "Updater.h"
#include <WiFi.h>
#include <HTTPUpdate.h>

static bool startUpdate = false;
static WiFiClient otaClient;

void updaterStart() {
  startUpdate = true;
}

void updaterLoop() {
  if (!startUpdate) return;
  startUpdate = false;

  t_httpUpdate_return ret = httpUpdate.update(
    otaClient,
    "http://172.70.70.225/iot/server/DEV-IOT_SVR-001.bin"
  );

  switch (ret) {
    case HTTP_UPDATE_FAILED:
      Serial.printf(
        "OTA failed (%d): %s\n",
        httpUpdate.getLastError(),
        httpUpdate.getLastErrorString().c_str()
      );
      break;

    case HTTP_UPDATE_NO_UPDATES:
      Serial.println("OTA no updates");
      break;

    case HTTP_UPDATE_OK:
      Serial.println("OTA success, rebooting");
      break;
  }
}
