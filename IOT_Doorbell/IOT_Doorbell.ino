/*
 * IOT Doorbell Creates Pushbullet push when someone rings the bell.
 */

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>


#define WIFI_SSID "changeme"
#define WIFI_PASS "changeme"

#define PUSHBULLET_TOKEN "changeme" //Preferably use a sacrificial account because the recipient is not verified.

const char* host = "https://api.pushbullet.com/v2/pushes";
const int httpsPort = 443;

std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);
HTTPClient https;

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN,HIGH);
  pinMode(D1, INPUT_PULLUP);
  Serial.begin(115200);
  Serial.println();
  Serial.print("connecting to ");
  Serial.println(WIFI_SSID);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    digitalWrite(LED_BUILTIN, LOW);
    delay(500);
    Serial.print(".");
  }
  digitalWrite(LED_BUILTIN,HIGH);
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  String messagebody = "{\"type\": \"note\", \"title\": \"Doorbell\", \"body\": \"Doorbell is online\"}\r\n";
  push(messagebody);
}

bool push(const String& messagebody){
  //------------------------------------------------------------------------------------
  //Pushbullet fingerprint changes too often to hardcode it and verifying a chain is beyond my capabilities.
  //Note, that this might allow someone pretending to be pushbullet to intercept your messages.
  client->setInsecure();
  //------------------------------------------------------------------------------------
  Serial.println("HTTPS BEGIN");
  digitalWrite(LED_BUILTIN, LOW);
  if(https.begin(*client, host)){
    Serial.println("POST");
    https.addHeader("Content-Type", "application/json",false, true);
    https.addHeader("Access-Token", PUSHBULLET_TOKEN, false, true);
    int httpCode = https.POST(messagebody);
    if(httpCode > 0){
      digitalWrite(LED_BUILTIN,HIGH);
      Serial.println("POST SUCCESS");
    } else {
      Serial.println("POST FAILURE");
      digitalWrite(LED_BUILTIN, LOW);
      return false;
    }
  } else {
    digitalWrite(LED_BUILTIN,LOW);
    Serial.println("HTTPS connection failed");
    return false;
  }
  return true;
}

long lastUpdate = 0;

int clientNum = 0;

bool lastTurnPressed = false;

bool isRinging(){
  for(int i=0; i<5;++i){
    if(digitalRead(D1) == LOW){
      return true;  
    }
    delay(5);
  }
  return false;
}

void loop() {
  if(clientNum > 0){
    String messagebody;
    //Modify as needed to notify multiple people
    switch(clientNum){
      case 1:
        messagebody = "{\"type\": \"note\", \"title\": \"Doorbell\", \"body\": \"Someone rang!\"}\r\n";
      break;
    }
    if(push(messagebody)){
      clientNum--;
    }
  } else {
    long curr = millis();
    if(curr - lastUpdate > 10){
      lastUpdate = curr;
      if(lastTurnPressed){
        if(!isRinging()){
          lastTurnPressed = false;
        }
      } else {
        if(isRinging()) {
          lastTurnPressed = true;
          clientNum = 1;
          Serial.println("Doorbell rang");
        }
      }
    }
  }
}
