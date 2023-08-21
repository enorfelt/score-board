#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>

AsyncWebServer server(80);

void setup()
{
  Serial.begin(115200);

  WiFi.begin("LMV19", "NorfWiks");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println("Connected to WiFi " + WiFi.localIP().toString());

  server.serveStatic("/", LittleFS, "/www/")
      .setDefaultFile("index.html")
      .setCacheControl("max-age=600");

  server.begin();
}

void loop()
{
  // put your main code here, to run repeatedly:
}
