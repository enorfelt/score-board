#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <LittleFS.h>
#include <ScoreBoardState.h>
#include <ScoreBoardCom.h>

class ScoreBoardServer
{
public:
  ScoreBoardServer(AsyncWebServer *server, ScoreBoardStateStore *stateStore);
  void Start();

private:
  AsyncWebServer *server;
  ScoreBoardStateStore *stateStore;
};