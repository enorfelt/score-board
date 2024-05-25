#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include <FS.h>

AsyncWebServer server(5000);

void setup()
{
  Serial.begin(9600);

  Serial.println("HTTP server starting..");

  WiFi.begin("","");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println("Connected to WiFi " + WiFi.localIP().toString());

  if (!LittleFS.begin())
  {
    Serial.println("An Error has occurred while mounting LittleFS");
    return;
  }

  server.serveStatic("/", LittleFS, "/browser")
      .setDefaultFile("index.html")
      .setCacheControl("max-age=600");

  server.on("/hello", HTTP_GET, [](AsyncWebServerRequest *request) {
    Serial.println("Hello World");
    request->send(200, "text/plain", "Hello World!");
  });

  server.onNotFound([](AsyncWebServerRequest *request) {
    Serial.printf("Not found: %s\n", request->url().c_str());
    request->send(404, "text/plain", "Not found");
  });

  server.begin();

  Serial.println("HTTP server started");
}

void loop()
{
  // put your main code here, to run repeatedly:
}
