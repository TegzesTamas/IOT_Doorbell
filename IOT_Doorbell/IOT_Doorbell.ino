/*
 * IOT Doorbell Creates Pushbullet push when someone rings the bell.
 * Based very much on code found here: http://www.esp8266.com/viewtopic.php?f=29&t=7116
 * 
 */

#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>

#define WIFI_SSID "changeme"
#define WIFI_PASS "changeme"

#define PUSHBULLET_TOKEN "changeme"

const char* host = "api.pushbullet.com";
const int httpsPort = 443;

// Use web browser to view and copy
// SHA1 fingerprint of the certificate
const char* fingerprint = "2C BC 06 10 0A E0 6E B0 9E 60 E5 96 BA 72 C5 63 93 23 54 B3"; //got it using https://www.grc.com/fingerprints.htm

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.print("connecting to ");
  Serial.println(WIFI_SSID);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  String messagebody = "{\"type\": \"note\", \"title\": \"Doorbell\", \"body\": \"Doorbell is online\"}\r\n";
  push(messagebody);
}

void push(const String& messagebody){
  WiFiClientSecure client;
  Serial.print("connecting to ");
  Serial.println(host);
  while (!client.connect(host, httpsPort)) {
    Serial.println("connection failed");
  }

  if (client.verify(fingerprint, host)) {
    Serial.println("certificate matches");
  } else {
    Serial.println("certificate doesn't match");
  }
  String url = "/v2/pushes";
  Serial.print("requesting URL: ");
  Serial.println(url);

  client.print(String("POST ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "Authorization: Bearer " + PUSHBULLET_TOKEN + "\r\n" +
               "Content-Type: application/json\r\n" +
               "Content-Length: " +
               String(messagebody.length()) + "\r\n\r\n");
  client.print(messagebody);



  Serial.println("request sent");

  //print the response

  while (client.available() == 0);

  while (client.available()) {
    String line = client.readStringUntil('\n');
    Serial.println(line);
  }
}

void loop() {
  if(digitalRead(2) == LOW){
    Serial.println("Someone rang! Pushing bullet!");
    String messagebody = "{\"type\": \"note\", \"title\": \"Doorbell\", \"body\": \"Someone rang!\"}\r\n";
    push(messagebody);
    while(digitalRead(2) == LOW);
  }
  delay(100);
}
