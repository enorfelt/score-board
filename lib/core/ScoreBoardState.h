#ifndef SCOREBOARDSTATE_H
#define SCOREBOARDSTATE_H

#include <memory>
#include <string>
#include <ScoreBoardCom.h>

// SoftwareSerial RX/TX pins for the scoreboard hardware connection.
// Override per environment via build_flags in platformio.ini:
//   -DSCOREBOARD_RX_PIN=<gpio> -DSCOREBOARD_TX_PIN=<gpio>
// ESP8266 D1 Mini default: D5 (GPIO14) / D6 (GPIO12)
// ESP32-S3 default: GPIO5 / GPIO6  — verify against your actual wiring
#ifndef SCOREBOARD_RX_PIN
  #ifdef ARDUINO_ARCH_ESP8266
    #define SCOREBOARD_RX_PIN 14  // D5 on ESP8266 D1 Mini
  #else
    #define SCOREBOARD_RX_PIN 5
  #endif
#endif
#ifndef SCOREBOARD_TX_PIN
  #ifdef ARDUINO_ARCH_ESP8266
    #define SCOREBOARD_TX_PIN 12  // D6 on ESP8266 D1 Mini
  #else
    #define SCOREBOARD_TX_PIN 6
  #endif
#endif

struct ScoreBoardState
{
  int home = 0;
  int away = 0;
  int inning = 1;
  int outsInInning = 0;
};

struct UpdateStateResult
{
  bool success;
  std::string message;
};

class ScoreBoardStateStore
{
public:
  ScoreBoardStateStore()
  {
    com = std::make_unique<ScoreBoardCom>(SCOREBOARD_RX_PIN, SCOREBOARD_TX_PIN);
  }
  ~ScoreBoardStateStore()
  {
    com->Close();
  }
  ScoreBoardState& getState() { return scoreBoardState; }
  UpdateStateResult updateState(const ScoreBoardState &newState)
  {
    UpdateStateResult result = UpdateAll(newState);
    if (result.success)
    {
      scoreBoardState = newState;
    }
    return result;
  }
  bool isReady() {
    return this->_isReady;
  }
  void begin();

private:
  bool _isReady = false;
  ScoreBoardState scoreBoardState;
  std::unique_ptr<ScoreBoardCom> com;
  UpdateStateResult UpdateAll(const ScoreBoardState &newState) const;
  bool sendCommandAndCheckResult(const char* command, UpdateStateResult& result) const;
};

#endif // SCOREBOARDSTATE_H