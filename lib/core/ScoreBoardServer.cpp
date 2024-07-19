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
  state.home = json["home"];
  state.away = json["away"];
  state.inning = json["inning"];
  state.outsInInning = json["outsInInning"];
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
  server->serveStatic("/", LittleFS, "/browser/")
      .setDefaultFile("index.html")
      .setLastModified("Mon, 15 Jul 2024 00:00:00 GMT");

  AsyncCallbackJsonWebHandler *updateHandler = new AsyncCallbackJsonWebHandler("/api/score-board/update", [this](AsyncWebServerRequest *request, JsonVariant &json)
                                                                               {
  const JsonObject& bodyObj = json.as<JsonObject>();
  const JsonObject& payloadObj = bodyObj["payload"].as<JsonObject>();

  std::unique_ptr<ScoreBoardState> scoreBoardState = std::make_unique<ScoreBoardState>(JsonToState(payloadObj));
  UpdateStateResult result = stateStore->updateState(*scoreBoardState);

  if (!result.success)
  {
    request->send(500, F("application/json"), F("{\"message\":\"") + result.message + F("\"}"));
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

  stateStore->begin();
}
