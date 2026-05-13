#include <ScoreBoardServer.h>
#include <AsyncJson.h>
#include <time.h>

String StateToJson(const ScoreBoardState &state)
{
  JsonDocument doc;
  doc["home"] = state.home;
  doc["away"] = state.away;
  doc["inning"] = state.inning;
  doc["outsInInning"] = state.outsInInning;

  String response;
  serializeJson(doc, response);

  return response;
}

ScoreBoardState JsonToState(const JsonObject &json)
{
  ScoreBoardState state;
  state.home = constrain((int)json["home"], 0, 99);
  state.away = constrain((int)json["away"], 0, 99);
  state.inning = constrain((int)json["inning"], 1, 9);
  state.outsInInning = constrain((int)json["outsInInning"], 0, 2);
  return state;
}

void sendJsonResponse(AsyncWebServerRequest *request, const ScoreBoardState &state, int statusCode)
{
  request->send(statusCode, F("application/json"), StateToJson(state));
}

ScoreBoardServer::ScoreBoardServer(AsyncWebServer *server, ScoreBoardStateStore *stateStore) : server(server), stateStore(stateStore)
{
}

void ScoreBoardServer::Start()
{
  // Compute ETag from index.html content (djb2 hash) so it changes on every uploadfs
  {
    File f = LittleFS.open(F("/browser/index.html"), "r");
    if (f)
    {
      uint32_t hash = 5381;
      while (f.available())
      {
        hash = ((hash << 5) + hash) + (uint8_t)f.read();
      }
      f.close();
      indexEtag = String(F("\"")) + String(hash, HEX) + F("\"");
    }
    else
    {
      indexEtag = F("\"0\"");
    }
  }

  // Serve index.html with no-cache + ETag; registered before serveStatic so it takes priority
  auto serveIndex = [this](AsyncWebServerRequest *request)
  {
    if (request->hasHeader("If-None-Match") && request->header("If-None-Match") == indexEtag)
    {
      request->send(304);
      return;
    }
    AsyncWebServerResponse *response =
        request->beginResponse(LittleFS, F("/browser/index.html"), F("text/html"));
    response->addHeader(F("Cache-Control"), F("no-cache"));
    response->addHeader(F("ETag"), indexEtag);
    request->send(response);
  };

  server->on("/", HTTP_GET, serveIndex);
  server->on("/index.html", HTTP_GET, serveIndex);

  server->serveStatic("/", LittleFS, "/browser/")
      .setLastModified("Fri, 19 Jul 2024 00:00:00 GMT");

  AsyncCallbackJsonWebHandler *updateHandler = new AsyncCallbackJsonWebHandler("/api/score-board/update", [this](AsyncWebServerRequest *request, JsonVariant &json)
                                                                               {
  const JsonObject& bodyObj = json.as<JsonObject>();
  if (!bodyObj["payload"].is<JsonObject>())
  {
    request->send(400, F("application/json"), F("{\"message\":\"Missing or invalid payload\"}"));
    return;
  }
  const JsonObject& payloadObj = bodyObj["payload"].as<JsonObject>();

  std::unique_ptr<ScoreBoardState> scoreBoardState = std::make_unique<ScoreBoardState>(JsonToState(payloadObj));
  UpdateStateResult result = stateStore->updateState(*scoreBoardState);

  if (!result.success)
  {
    request->send(500, F("application/json"), String(F("{\"message\":\"")) + result.message.c_str() + F("\"}"));
    return;
  }

  sendJsonResponse(request, *scoreBoardState, 200); });

  server->addHandler(updateHandler);

  server->on("/api/score-board/load", HTTP_GET, [this](AsyncWebServerRequest *request)
             { sendJsonResponse(request, stateStore->getState(), 200); });

  const String isReadyTrue = F("{ \"isReady\": true }");
  const String isReadyFalse = F("{ \"isReady\": false }");
  server->on("/api/score-board/status", HTTP_GET, [this, isReadyTrue, isReadyFalse](AsyncWebServerRequest *request)
             { request->send(200, F("application/json"), stateStore->isReady() ? isReadyTrue : isReadyFalse); });

  server->on("/api/score-board/start", HTTP_GET, [this, isReadyTrue, isReadyFalse](AsyncWebServerRequest *request)
             {
                stateStore->begin();
                request->send(200, F("application/json"), stateStore->isReady() ? isReadyTrue : isReadyFalse); });

  server->onNotFound([](AsyncWebServerRequest *request)
                     { request->send(404, F("text/plain"), F("Not found")); });

  server->begin();
}
