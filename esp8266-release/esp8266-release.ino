/*!
 * ESP8266-RemoteRelease v0.1
 * Copyright 2017 kat-kai
 * Licensed under the MIT license
 */

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <FS.h>
#include <Ticker.h>

#define SHUTTER_PIN 16
#define FOCUS_PIN 14

ESP8266WebServer server(80);
Ticker ticker, subticker;

void connectWiFi() {
  WiFi.disconnect();
  delay(10);
  
  WiFi.mode(WIFI_STA);
  WiFi.softAP("esp-release", "password");

  Serial.println();
  Serial.print("softAP IP address: ");
  Serial.println(WiFi.softAPIP()); //Default ESP8266's IP: 192.168.4.1
}

void getLocalFile(String filename, String dataType) {
  SPIFFS.begin();
  File data = SPIFFS.open(filename, "r");
  server.streamFile(data, dataType);
  data.close();
}

void releaseButton(int pin, int msec){
  digitalWrite(pin, HIGH);
  delay(msec);
  digitalWrite(pin, LOW);
}

void setup(void){
  Serial.begin(115200);

  pinMode(SHUTTER_PIN, OUTPUT);
  pinMode(FOCUS_PIN, OUTPUT);

  connectWiFi();
  
  server.on("/", [](){ getLocalFile("/index.html", "text/html"); });
  server.on("/bootstrap.min.js", [](){ getLocalFile("/bootstrap.min.js", "text/javascript"); });
  server.on("/bootstrap.min.css", [](){ getLocalFile("/bootstrap.min.css", "text/css"); });

  server.on("/on/", [](){ Serial.println("on"); digitalWrite(SHUTTER_PIN, HIGH); server.send(200, "text/html", ""); });
  server.on("/off/", [](){ Serial.println("off"); ticker.detach(); digitalWrite(SHUTTER_PIN, LOW); server.send(200, "text/html", ""); });

  server.on("/shutter/", [](){ Serial.println("shutter"); releaseButton(SHUTTER_PIN, 1000); server.send(200, "text/html", ""); }); 
  server.on("/focus/", [](){ Serial.println("focus"); releaseButton(FOCUS_PIN, 1000); server.send(200, "text/html", ""); });

  server.on("/interval/", [](){
    Serial.print("interval: ");
    Serial.println(server.arg("t"));
    
    int wait_ms = server.arg("t").toInt() * 1000;
    ticker.attach_ms(wait_ms, [](){
      Serial.println("interval");
      digitalWrite(SHUTTER_PIN, HIGH);
      subticker.once_ms(1000, [](){ digitalWrite(SHUTTER_PIN, LOW); });
      });
    server.send(200, "text/html", "");
  });

  server.on("/self/", [](){
    Serial.print("self: ");
    Serial.println(server.arg("t"));
    
    int wait_ms = server.arg("t").toInt() * 1000;
    ticker.once_ms(wait_ms, [](){
      Serial.println("self");
      digitalWrite(SHUTTER_PIN, HIGH);
      subticker.once_ms(1000, [](){ digitalWrite(SHUTTER_PIN, LOW); });
      });
    server.send(200, "text/html", "");
  });


  server.onNotFound([](){ server.send(404, "text/plain", "File Not Found"); });

  server.begin();
  Serial.println("httpd: Started.");

}

void loop(void){
  server.handleClient();
}
