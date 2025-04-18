#include <WiFi.h>
#include <PubSubClient.h>
#include <HTTPClient.h>
#include <ESP32httpUpdate.h>
#include <ModbusRTU.h>
#include <esp_task_wdt.h>

#define SLAVE_ID1 1
#define SLAVE_ID2 2
#define FIRST_REG 0
#define REG_COUNT 2
#define MBUS_HW_SERIAL Serial1
#define MBUS_RXD_PIN 27
#define MBUS_TXD_PIN 13
#define REG_COUNT 2
ModbusRTU mb;

bool cb(Modbus::ResultCode event, uint16_t transactionId, void* data) {
  if (event != Modbus::EX_SUCCESS) {
    Serial.print("Request result: 0x");
    Serial.print(event, HEX);
  }
  return true;
}

const int sensor_fire_alarm = 36;
const int l_conn = 23;
unsigned long tb_1;
unsigned long f_ct;
unsigned long f_dt;
unsigned long stage_1;
unsigned long stage_2;
unsigned long lastSend1;
unsigned long s_a_t;

bool send_alarm_on = true;
bool send_alarm_off = true;

int count_send;
int count_temp_start;
int count_temp_stop;

float sv1 = 0;
float pv1 = 0;
float sv2 = 0;
float pv2 = 0;

String IP_ID = "DEV-IOT_SVR-001";
String id_node = "";
String firmware = "0.16";

const char* ssid = "TT-IOT";
const char* password = "ttiot2019"; 

IPAddress local_IP(172, 70, 42, 41);
IPAddress gateway(172, 70, 42, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress primaryDNS(172, 70, 11, 98);
IPAddress secondaryDNS(172, 70, 11, 99);
IPAddress server(192, 168, 10, 77);

String user_mqtt = "tt_mqtt";
String pass_mqtt = "12345678";

char buf[300];
char message_buff[100];
String msgString = "";
String data[50];
int status_con_lamp = LOW;
int try_conn = 0;
int send_c = 0;
bool disc_n = false; bool conn_n = false; bool alert = true;
float temp;
int count_t = 0;

void callback(char* topic, byte* payload, unsigned int length);
void setup_wifi();
void reconnect_mqtt();
void parsingData(String indata);
void updater();
void req_mod(); 
void status_mqtt(String type);

WiFiClient wificlient;
PubSubClient client(wificlient);

void setup () {
  pinMode(sensor_fire_alarm, INPUT);
  pinMode(l_conn, OUTPUT);

  MBUS_HW_SERIAL.begin(9600, SERIAL_8N1, MBUS_RXD_PIN, MBUS_TXD_PIN);
  mb.begin(&MBUS_HW_SERIAL);
  mb.master();

  id_node = IP_ID + String(random(0xfffff), HEX);
  Serial.begin(9600);
  client.setServer(server, 1883);
  client.setCallback(callback);
  WiFi.disconnect();
  delay(1000);
  setup_wifi();
  reconnect_mqtt();
  esp_task_wdt_init(1, true);
}

void loop() {
  esp_task_wdt_reset();
  client.loop();
  if (!client.connected()) reconnect_mqtt();
  if (WiFi.status() != WL_CONNECTED) setup_wifi();
  if (millis() - lastSend1 > 1000) {
    lastSend1 = millis();
    req_mod();
    status_mqtt("RT");
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  int i = 0;
  for (i = 0; i < length; i++){
    message_buff[i] = payload[i];
  }
  message_buff[i] = '\0';
  msgString = String(message_buff);
  parsingData(msgString);
  msgString = "";
}

void setup_wifi(){
  delay(10);
  Serial.printf("Connecting to %s ", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    status_con_lamp = not (status_con_lamp);
    digitalWrite(l_conn, status_con_lamp);
    Serial.print(".");
    delay(500);
    try_conn++;
    
    if (try_conn == 20) {
      digitalWrite(l_conn, LOW);
      while (WiFi.waitForConnectResult() != WL_CONNECTED) {
        delay(3000);
        ESP.restart();
      }
    }
  }

  if (WiFi.status() == WL_CONNECTED) {
    try_conn = 0;

    if (conn_n) {
      digitalWrite(l_conn, HIGH);
      randomSeed(micros());
      Serial.println(WiFi.localIP());
      conn_n = false;
    }
  }
}

void req_mod () {
  uint16_t res1[REG_COUNT];
  if (!mb.slave()) {
    mb.readIreg(SLAVE_ID1, FIRST_REG, REG_COUNT, res1, cb);
    while (mb.slave()) {
      mb.task();
    }
  }
  pv1 = res1[0];
  sv1 = res1[1];
  pv2 = 0;
  sv2 = 0; 
}

void reconnect_mqtt() 
{
  if (client.connect(id_node.c_str(), pass_mqtt.c_str())) 
  {
    String json_strg = "{\"type\":\"STS\",\"id\":\"" + IP_ID + "\",\"firmware\":\"" + firmware + "\",\"Alarm\":\"" + alert + "\",\"RSSI\":\"" + WiFi.RSSI() + "\"}";
    char *msg_mqtt = const_cast<char*>(json_strg.c_str());
    client.publish("IOT/DEV/TEMP/SERVER1", msg_mqtt);
    client.subscribe(IP_ID.c_str());
  }
}

void status_mqtt(String type)
{
  if (client.connect(id_node.c_str(),user_mqtt.c_str(), pass_mqtt.c_str()))
  {
    String json_strg = "{\"IP_ID\":\"" + IP_ID + "\",\"type\":\"" + type + "\",\"vt\":\"0\",\"PV1\":\"" + pv1 + "\",\"SV1\":\"" + sv1 + "\",\"PV2\":\"" + pv2 + "\",\"SV2\":\"" + sv2 + "\"}";
    char *msg_mqtt = const_cast<char*>(json_strg.c_str());
    client.publish("IOT/SERVER/TEMP1", msg_mqtt);
    client.subscribe(IP_ID.c_str());
  }

  int s_t = pv1;
  int s_h = sv1;
  int u_t = pv2;
  int u_h = sv2;

 if (client.connect(id_node.c_str(),user_mqtt.c_str(), pass_mqtt.c_str()))
  {
    String json_strg = "{\"type\":\"" + type + "\",\"PV1\":\"" + s_t + "\",\"SV1\":\"" + s_h + "\",\"PV2\":\"" + u_t + "\",\"SV2\":\"" + u_h + "\"}";
    char *msg_mqtt = const_cast<char*>(json_strg.c_str());
    client.publish("DEV-IOT-DISPLAY-001", msg_mqtt);
    client.subscribe(IP_ID.c_str());
  }
}

void trans_mqtt_alarm(String triger, String value)
{
  if (client.connect(id_node.c_str(),user_mqtt.c_str(), pass_mqtt.c_str()))
  {
    String json_strg = "{\"type\":\"Alarm\",\"triger\":\"" + triger + "\",\"value\":\"" + value + "\"}";
    char *msg_mqtt = const_cast<char*>(json_strg.c_str());
    client.publish("DEV-IOT-DISPLAY-001", msg_mqtt);
    client.subscribe(IP_ID.c_str());
  }
}

 void parsingData (String indata) {
  int count = 0, h =1;
  int early, end;
  char my_array[10];
  String b;
  for (int i = 0; i< indata.length(); i++) {
    if (indata[i] == '{')
    {
      early = i;
    }
    else if (indata[i] == '}')
    end = i;
    break;
  }

  for (int i = early; i < end; i++) {
    if (indata[i] == '{' || indata[i] == '}') {
      count++;
      data[count] = "";
    }
    else {
      data[count] = data[count] + indata[i];
    }
  }
  if (data[1] == "Update")  {
    updater();
  }
  else if (data[1] == "Ping")
  {
    reconnect_mqtt();
  }
  else if (data[1] == "Reboot")
  {
    delay(1000);
    ESP.restart();
  }
  else if (data[1] == "Disable")
  {
    alert = false;
    trans_mqtt_alarm("temp","stop");
  }
   else if (data[1] == "Enable")
  {
    alert = true;
  }
 } 

void updater() {
  for (uint8_t t = 4; t > 0; t--) {
    Serial.printf("[SETUP] WAIT %d...\n", t);
    Serial.flush();
    for (int x = 0; x < 20; x++)
    {
//      digitalWrite(s_tag, LOW);
//      digitalWrite(l_conn, HIGH);
      delay(50);
//      digitalWrite(s_tag, HIGH);
//      digitalWrite(l_conn, LOW);
      delay(50);
    }
    //    delay(1000);
  }

  if (WiFi.status() == WL_CONNECTED)
  {
    t_httpUpdate_return ret = ESPhttpUpdate.update("http://172.70.70.225/iot/server/" + IP_ID + ".bin");

    switch (ret) {
      case HTTP_UPDATE_FAILED:
        //        Serial.printf("HTTP_UPDATE_FAILD Error (%d): %s", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
        //        mode_update("HTTP_UPDATE_FAILD Error");
        break;

      case HTTP_UPDATE_NO_UPDATES:
        //        Serial.println("HTTP_UPDATE_NO_UPDATES");
        //        mode_update("HTTP_UPDATE_NO_UPDATES");
        break;

      case HTTP_UPDATE_OK:
        //        Serial.println("HTTP_UPDATE_OK");
        //        mode_update("HTTP_UPDATE_OK");
        break;
    }

  }
}
