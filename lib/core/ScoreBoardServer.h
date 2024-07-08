#include <ESPAsyncWebServer.h>
#include <AsyncJson.h>
#include <ArduinoJson.h>
#include <LittleFS.h>

struct UpdateResult
{
  bool success;
  String message;
};

class ScoreBoardServer
{
public:
  ScoreBoardServer(AsyncWebServer *server);
  void Start();

private:
  AsyncWebServer *server;
};