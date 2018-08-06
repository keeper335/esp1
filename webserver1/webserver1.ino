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

//#include "html_page.h"
#include "ws1_API.h"

const char *ESP_CONFIG_FILE = "config";
char *INDEX_HTML;
static const char NOTFOUND_HTML[] = R"rawliteral(<h1>Ooops</h1><p>Someone destroyed the page you are looking for</p>)rawliteral";

int wifi_mode = WIFI_AP; //WIFI_STA = 1, WIFI_AP = 2
String ssid = "KEEPER_ESP1";
String password = "Gokeeper0#";
String mIp_string = "192.168.1.160";
long mIp_port = 80;
long mSocket_port = 81;
ESP8266WebServer *server;
WebSocketsServer *webSocket;

int sdcard_init();
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length);

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
    WiFi.config(mIp, mgw, msubnet);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.print(".");
    }
  }

  Serial.print("WiFi initialized: ");
  Serial.println(WiFi.localIP());
  return 0;
}

void server_init() {
  Serial.println("Initialize web server");
  server = new ESP8266WebServer(mIp_port);
  delay(500);
  server->on("/", [](){server->send(200, "text/html", INDEX_HTML);});
  server->onNotFound([]() {server->send(404, "text/plain", NOTFOUND_HTML);});
  server->begin();
  Serial.println("HTTP server started");


  Serial.println("Initialize web socket");
  webSocket = new WebSocketsServer(mSocket_port);
  webSocket->begin();
  webSocket->onEvent(webSocketEvent);
}


void setup() {
  // put your setup code here, to run once:
  pinMode(2, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(0, OUTPUT);
  
  digitalWrite(2, HIGH);
  digitalWrite(5, HIGH);
  digitalWrite(4, HIGH);
  digitalWrite(0, HIGH);

  
  Serial.begin(74880);
  
  while(!Serial){;}
  Serial.println("\nSerial Ok");
   

  //Init SD card and read config
  if (sdcard_init() < 0) return;
  //Init wifi
  if (wifi_init() < 0) return;
  server_init();

  //write_config();
}

void loop() {
  // put your main code here, to run repeatedly:
  webSocket->loop();
  server->handleClient();
  //if (intled_flash_ > 0)
//    flash_internal_led();
  //else intled_flash_counter = 0;
}

void write_config() {
  Serial.println("Write config file");
  StaticJsonBuffer<500> jsonBuffer;
  File configFile;
  
  JsonObject& root = jsonBuffer.createObject();
  root["wifi_mode"] = 2; //WIFI_AP
  root["ip_address"] = "192.168.1.160";
  root["ip_port"] = 3000;
  root["socket_port"] = 3025;
  //root[""] = ;
  //root[""] = ;

  SD.remove(ESP_CONFIG_FILE);
  
  configFile = SD.open(ESP_CONFIG_FILE, FILE_WRITE);
  if (!configFile) {
    Serial.println("Failed opening config file");
    return;
  }

  if (root.printTo(configFile) == 0) {
    Serial.println("Failed to write config file");
  }

  configFile.close();
}

int sdcard_init() {
  File configFile, indexFile;
  //String config_ = "";
  StaticJsonBuffer<512> jsonBuffer;
  
  Serial.print("\nInitializing SD card...");
  if (!SD.begin(15)) {
    Serial.println("initialization failed");
    return -1;
  }
  Serial.println("Wiring is correct and a card is present.");

  configFile = SD.open(ESP_CONFIG_FILE, FILE_READ);
  //if (configFile) {
    //while(configFile.available()) config_+=configFile.readString();
  //} else {
    //Serial.println("Failed reading config file");
    //return -1;
  //}
  
  
  JsonObject& root = jsonBuffer.parseObject(configFile);
  if (root.success()) {
    root.printTo(Serial);
    Serial.println("");
    
    String mIp_string2 = root["ip_address"];
    String wifi_ssid = root["wifi_ssid"];
    String wifi_pswd = root["wifi_password"];
    int wifi_mode2 = root["wifi_mode"];
    long mIp_port2 = root["ip_port"];
    long mSocket_port2 = root["socket_port"];
    if (mIp_string2) mIp_string = mIp_string2;
    if (mIp_port2 > 0) mIp_port = mIp_port2;
    if (mSocket_port2 > 0) mSocket_port = mSocket_port2;
    if (wifi_mode2 > 0) wifi_mode = wifi_mode2;

    if (wifi_ssid) ssid = wifi_ssid;
    if (wifi_pswd) password = wifi_pswd;

    //Serial.println(config_);
  }
  else {
    Serial.println("Failed parsing config object");
    
  }

  configFile.close();


  indexFile = SD.open("index.htm");
  if(indexFile) {
    //while(indexFile.available()) {
    //  Serial.write(indexFile.read());
    //}
    //Serial.println("");

uint32_t indexSize = indexFile.size();
Serial.println("Allocate string for bytes : " + String(indexSize));
INDEX_HTML = new char[indexSize];
if (!INDEX_HTML) Serial.println("Failed allocating mem");
int readBytes = indexFile.read(INDEX_HTML, indexSize);
Serial.println("Read index file bytes : " + String(readBytes));

    indexFile.close();
  }
  else
    Serial.println("Cannot open index html");
  //
  //
  //
  //
  //
  //

  
  
  return 0;
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length)
{
  static char backMessage[128];
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
      memset(backMessage, 0, 128 * sizeof(char));
      handleWebSocketAPI((const char *)payload, backMessage);

      webSocket->broadcastTXT(backMessage, strlen(backMessage));
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


