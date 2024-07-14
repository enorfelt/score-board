#include <ScoreBoardServer.h>
#include <AsyncJson.h>

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
  String response = StateToJson(state);
  Serial.println("Sending response: " + response);
  request->send(statusCode, "application/json", response);
}

ScoreBoardServer::ScoreBoardServer(AsyncWebServer *server) : server(server), stateStore(new ScoreBoardStateStore())
{
}

void ScoreBoardServer::Start()
{
  server->serveStatic("/", LittleFS, "/browser/")
      .setDefaultFile("index.html")
      .setCacheControl("max-age=600");

  AsyncCallbackJsonWebHandler *updateHandler = new AsyncCallbackJsonWebHandler("/api/score-board/update", [this](AsyncWebServerRequest *request, JsonVariant &json)
                                                                               {
  JsonObject bodyObj = json.as<JsonObject>();
  JsonObject payloadObj = bodyObj["payload"].as<JsonObject>();

  ScoreBoardState scoreBoardState = JsonToState(payloadObj);
  UpdateStateResult result = stateStore->updateState(scoreBoardState);

  if (!result.success)
  {
    request->send(500, "application/json", "{\"message\":\"" + result.message + "\"}");
    return;
  }

  sendJsonResponse(request, scoreBoardState, 200); });

  server->addHandler(updateHandler);

  server->on("/api/score-board/load", HTTP_GET, [this](AsyncWebServerRequest *request)
             { sendJsonResponse(request, stateStore->getState(), 200); });

  server->on("/api/score-board/status", HTTP_GET, [this](AsyncWebServerRequest *request)
             {
                String isReady = stateStore->isReady() ? "true" : "false"; 
                request->send(200, "application/json", "{ \"isReady\": " + isReady + " }"); 
              });

  server->onNotFound([](AsyncWebServerRequest *request)
                     {
    Serial.printf("Not found: %s\n", request->url().c_str());
    request->send(404, "text/plain", "Not found"); });

  server->begin();

  stateStore->begin();
}
