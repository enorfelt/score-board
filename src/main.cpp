#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <AsyncJson.h>
#include <ArduinoJson.h>
#include <LittleFS.h>
#include <FS.h>
#include <ScoreBoardState.h>
#include <ScoreBoardCom.h>

AsyncWebServer server(80);

ScoreBoardState scoreBoardState = {.home = 0, .away = 0, .inning = 1, .outsInInning = 0};
ScoreBoardCom compcomm(12, 14);

const char *MESSAGE_PASS = "Ok";

struct UpdateResult
{
  bool success;
  String message;
};

AsyncCallbackJsonWebHandler *updateHandler = new AsyncCallbackJsonWebHandler("/api/score-board/update", [](AsyncWebServerRequest *request, JsonVariant &json)
                                                                             {
  JsonObject bodyObj = json.as<JsonObject>();
  JsonObject payloadObj = bodyObj["payload"].as<JsonObject>();
  scoreBoardState.home = payloadObj["home"];
  scoreBoardState.away = payloadObj["away"];
  scoreBoardState.inning = payloadObj["inning"];
  scoreBoardState.outsInInning = payloadObj["outsInInning"];

  String response;
  serializeJson(payloadObj, response);
  Serial.println("Received update: " + response);
  request->send(200, "application/json", response); });

void setup()
{
  Serial.begin(115200);

  Serial.println("HTTP server starting..");

  WiFi.begin("", "");
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

  server.serveStatic("/", LittleFS, "/browser/")
      .setDefaultFile("index.html")
      .setCacheControl("max-age=600");

  server.addHandler(updateHandler);

  server.on("/api/score-board/load", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    JsonDocument doc;
    doc["home"] = scoreBoardState.home;
    doc["away"] = scoreBoardState.away;
    doc["inning"] = scoreBoardState.inning;
    doc["outsInInning"] = scoreBoardState.outsInInning;

    String response;
    serializeJson(doc, response);
    Serial.println("Sending load: " + response);
    request->send(200, "application/json", response); });

  server.onNotFound([](AsyncWebServerRequest *request)
                    {
    Serial.printf("Not found: %s\n", request->url().c_str());
    request->send(404, "text/plain", "Not found"); });

  server.begin();

  compcomm.Open();

  Serial.println("HTTP server started");
}

void loop()
{
  // put your main code here, to run repeatedly:
}

UpdateResult updateAll()
{
  UpdateResult result;
  result.success = true;
  String s;
  int i;

  // Home
  i = scoreBoardState.home / 10;
  s = "wd 0 " + String(i);
  if (!compcomm.SendCommandLookForString(s, MESSAGE_PASS))
  {
    result.success = false;
    result.message = "Unexpected answer from unit. Command was " + s + ". Was looking for string " + MESSAGE_PASS + " in output " + compcomm.fetchedOutputString;
    return result;
  }

  s = "wd 1 " + String(scoreBoardState.home - (10 * i));
  if (!compcomm.SendCommandLookForString(s, MESSAGE_PASS))
  {
    result.success = false;
    result.message = "Unexpected answer from unit. Command was " + s + ". Was looking for string " + MESSAGE_PASS + " in output " + compcomm.fetchedOutputString;
    return result;
  }

  // Innings
  s = "wd 2 " + String(scoreBoardState.inning);
  if (!compcomm.SendCommandLookForString(s, MESSAGE_PASS))
  {
    result.success = false;
    result.message = "Unexpected answer from unit. Command was " + s + ". Was looking for string " + MESSAGE_PASS + " in output " + compcomm.fetchedOutputString;
    return result;
  }

  // Guest
  i = scoreBoardState.away / 10;
  s = "wd 3 " + String(i);
  if (!compcomm.SendCommandLookForString(s, MESSAGE_PASS))
  {
    result.success = false;
    result.message = "Unexpected answer from unit. Command was " + s + ". Was looking for string " + MESSAGE_PASS + " in output " + compcomm.fetchedOutputString;
    return result;
  }

  s = "wd 4 " + String(scoreBoardState.away - (10 * i));
  if (!compcomm.SendCommandLookForString(s, MESSAGE_PASS))
  {
    result.success = false;
    result.message = "Unexpected answer from unit. Command was " + s + ". Was looking for string " + MESSAGE_PASS + " in output " + compcomm.fetchedOutputString;
    return result;
  }

  // Out
  if (( scoreBoardState.outsInInning > 0 && scoreBoardState.outsInInning < 3 ) || scoreBoardState.outsInInning > 3)
  {
    if (!compcomm.SendCommandLookForString("wb 5 0 1", MESSAGE_PASS))
    {
      result.success = false;
      result.message = "Unexpected answer from unit. Command was " + s + ". Was looking for string " + MESSAGE_PASS + " in output " + compcomm.fetchedOutputString;
      return result;
    }
  }
  else
  {
    if (!compcomm.SendCommandLookForString("wb 5 0 0", MESSAGE_PASS))
    {
      result.success = false;
      result.message = "Unexpected answer from unit. Command was " + s + ". Was looking for string " + MESSAGE_PASS + " in output " + compcomm.fetchedOutputString;
      return result;
    }
  }

  if (( scoreBoardState.outsInInning > 1 && scoreBoardState.outsInInning < 3 ) || scoreBoardState.outsInInning > 4)
  {
    if (!compcomm.SendCommandLookForString("wb 5 1 1", MESSAGE_PASS))
    {
      result.success = false;
      result.message = "Unexpected answer from unit. Command was " + s + ". Was looking for string " + MESSAGE_PASS + " in output " + compcomm.fetchedOutputString;
      return result;
    }
  }
  else
  {
    if (!compcomm.SendCommandLookForString("wb 5 1 0", MESSAGE_PASS))
    {
      result.success = false;
      result.message = "Unexpected answer from unit. Command was " + s + ". Was looking for string " + MESSAGE_PASS + " in output " + compcomm.fetchedOutputString;
      return result;
    }
  }
  return result;
}
