#include <ArduinoJson.h>
#include <WebSockets.h>
#include <WebSocketsClient.h>
#include <WebSocketsServer.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <SD.h>
/* SD reader < - > ESP 
 *  cs - d8
 *  mosi-d7
 *  sck -d5
 *  miso-d6
 */

#include "html_page.h"
#include "ws1_API.h"

#define led_pin 2
const String ESP_CONFIG_FILE = "config";


int wifi_mode = WIFI_AP; //WIFI_STA = 1, WIFI_AP = 2
String ssid = "KEEPER_ESP1";
String password = "Gokeeper0#";
String mIp_string = "192.168.1.160";
long mIp_port = 80;
long mSocket_port = 81;
ESP8266WebServer *server;
WebSocketsServer *webSocket;

int intled_flash_ = 0;
int intled_flash_counter = 0;
int intled_flash_state = LOW;

int sdcard_init();
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length);

void flash_internal_led() {
  //int state_ = digitalRead(led_pin);
  intled_flash_counter++;
  if (intled_flash_state == LOW && intled_flash_counter >= intled_flash_) {
    intled_flash_counter = 0;
    digitalWrite(led_pin, HIGH); //OFF
    intled_flash_state = HIGH;
  }
  else if (intled_flash_counter >= 100) {
    intled_flash_counter = 0;
    digitalWrite(led_pin, LOW); //ON
    intled_flash_state = LOW;
  }
}

int wifi_init() {
  int wifi_status = WL_IDLE_STATUS;
  IPAddress msubnet(255, 255, 255, 0);
  IPAddress mIp;
  IPAddress mgw;
  
  if (!mIp.fromString(mIp_string)) {
    Serial.println("Error parsing IP address from string");
  }
  if (!mgw.fromString(mIp_string)) {
    Serial.println("Error parsing Gateway address from string");
  }
    
  if (wifi_mode == WIFI_AP) {
    Serial.println("Initialize WiFi as Access point");
    WiFi.mode(WIFI_AP);
    WiFi.softAP(ssid.c_str(), password.c_str());
    WiFi.softAPConfig(mIp, mgw, msubnet);
  }
  else {
    Serial.println("Initialize WiFi as Station");
    //WIFI_STA
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid.c_str(), password.c_str());
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.print(".");
    }
  }

  Serial.print("WiFi initialized: ");
  Serial.println(WiFi.softAPIP());
  return 0;
}

void server_init() {
  Serial.println("Initialize web server");
  server = new ESP8266WebServer(mIp_port);
  delay(500);
  server->on("/", [](){server->send(200, "text/html", INDEX_HTML);});
  server->onNotFound([]() {server->send(404, "text/plain", NOTFOUND_HTML);});
  server->on("/submit", [](){
    String argName, argValue;
    
    int val_ = -1;
    Serial.print("Received submit event with data: ");
    for (byte i = 0; i < server->args(); i++) {
      argName = server->argName(i);
      Serial.print(argName);
      Serial.print("=");
      argValue = server->arg(i);
      Serial.print(argValue);
      

      if (argName.equals("state")) {
        if(argValue.equals("true")) val_ = HIGH;
        else if(argValue.equals("false")) val_ = LOW;
        Serial.print("! int ");
        Serial.print(val_);
      }

      Serial.print(", ");
    }
    Serial.println();
    
    if (val_ > -1)
      digitalWrite(led_pin, val_);
    String ret_ = "{\"state\":";
    ret_ += val_;
    ret_ += "}";
    server->send(200, "text/html", ret_);
  });
  server->begin();
  Serial.println("HTTP server started");


  Serial.println("Initialize web socket");
  webSocket = new WebSocketsServer(mSocket_port);
  webSocket->begin();
  webSocket->onEvent(webSocketEvent);
}


void setup() {
  // put your setup code here, to run once:
  //pinMode(led_pin, OUTPUT);
  //digitalWrite(led_pin, HIGH);
  Serial.begin(115200);
  
  while(!Serial){;}
  Serial.println("\nSerial Ok");
   

  //Init SD card and read config
  if (sdcard_init() < 0) return;
  //Init wifi
  if (wifi_init() < 0) return;
  server_init();
}

void loop() {
  // put your main code here, to run repeatedly:
  webSocket->loop();
  server->handleClient();
  if (intled_flash_ > 0)
    flash_internal_led();
  else intled_flash_counter = 0;
}

int sdcard_init() {
  File configFile;
  String config_ = "";
  StaticJsonBuffer<500> jsonBuffer;
  
  Serial.print("\nInitializing SD card...");
  if (!SD.begin(15)) {
    Serial.println("initialization failed");
    return -1;
  }
  Serial.println("Wiring is correct and a card is present.");

  configFile = SD.open(ESP_CONFIG_FILE, FILE_READ);
  if (configFile) {
    while(configFile.available()) config_+=configFile.readString();
    configFile.close();
  } else {
    Serial.println("Failed reading config file");
    return -1;
  }
  JsonObject& root = jsonBuffer.parseObject(config_);
  if (!root.success()) {
    Serial.println("Failed parsing config object");
    Serial.println(config_);
    root.printTo(Serial);
    return -1;
  }

  //Serial.print("Config IP address: ");
  String mIp_string2 = root["ip_address"];
  int wifi_mode2 = root["wifi_mode"];
  long mIp_port2 = root["ip_port"];
  long mSocket_port2 = root["socket_port"];
  if (mIp_string2) mIp_string = mIp_string2;
  if (mIp_port2 > 0) mIp_port = mIp_port2;
  if (mSocket_port2 > 0) mSocket_port = mSocket_port2;
  if (wifi_mode2 > 0) wifi_mode = wifi_mode2;

  return 0;
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length)
{
  Serial.printf("webSocketEvent(%d, %d, ...)\r\n", num, type);
  switch(type) {
    case WStype_DISCONNECTED:
      Serial.printf("[%u] Disconnected!\r\n", num);
      break;
    case WStype_CONNECTED:
      {
        IPAddress ip = webSocket->remoteIP(num);
        Serial.printf("Connected url: %s\r\n", payload);
      }
      break;
    case WStype_TEXT:
      Serial.printf("[%u] get Text: %s\r\n", num, payload);

      // send data to all connected clients
      webSocket->broadcastTXT(payload, length);

      if (strcmp(INTLEDON, (const char *)payload) ==0) {
        digitalWrite(led_pin, LOW);
        webSocket->broadcastTXT(INTLEDON_NTFY, strlen(INTLEDON_NTFY));
        intled_flash_ = 0;
      }
      else if (strcmp(INTLEDOFF, (const char *)payload) ==0) {
        digitalWrite(led_pin, HIGH);
        webSocket->broadcastTXT(INTLEDOFF_NTFY, strlen(INTLEDOFF_NTFY));
        intled_flash_ = 0;
      }
      else if (strcmp(INTLEDFLASH, (const char *)payload) ==0) {
        digitalWrite(led_pin, HIGH);
        intled_flash_ = 50;
        webSocket->broadcastTXT(INTLEDFLASH_NTFY, strlen(INTLEDFLASH_NTFY));
      }
      break;
    case WStype_BIN:
      Serial.printf("[%u] get binary length: %u\r\n", num, length);

      // echo data back to browser
      webSocket->sendBIN(num, payload, length);
      break;
    default:
      Serial.printf("Invalid WStype [%d]\r\n", type);
      break;
  }
}


