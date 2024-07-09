#include <Arduino.h>
#include <ESP8266WiFi.h> 
#include <LittleFS.h>
#include <FS.h>
#include <ScoreBoardServer.h>
#include <EspAsyncWiFiManager.h>
#include <ESP8266mDNS.h>

AsyncWebServer server(80);
DNSServer dns;

void setup()
{
  WiFi.mode(WIFI_STA);

  Serial.begin(115200);

  Serial.println("\n Starting");

  WiFi.hostname("ScoreBoard");

  AsyncWiFiManager wm(&server, &dns);

  bool res;

  res = wm.autoConnect("ScoreBoard Config");
  if (!res)
  {
    Serial.println("Failed to connect to WiFi");
    delay(3000);
    ESP.reset();
  }
  else
  {
    Serial.println("Connected to WiFi " + WiFi.localIP().toString());

    if (MDNS.begin("board")) {
      Serial.println("MDNS responder started");
      MDNS.addService("http", "tcp", 80);
    }

    if (!LittleFS.begin())
    {
      Serial.println("An Error has occurred while mounting LittleFS");
      return;
    }

    Serial.println("Staring HTTP server...");

    ScoreBoardServer scoreBoardServer(&server);
    scoreBoardServer.Start();

    Serial.println("HTTP server started");
  }
}

void loop()
{
  // put your main code here, to run repeatedly:
}

