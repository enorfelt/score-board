#ifndef SCOREBOARDSTATE_H
#define SCOREBOARDSTATE_H

#include <memory>
#include <ScoreBoardCom.h>

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
  String message;
};

class ScoreBoardStateStore
{
public:
  ScoreBoardStateStore()
  {
    com = std::make_unique<ScoreBoardCom>(12, 14);
    com->Open();
  }
  ~ScoreBoardStateStore()
  {
    com->Close();
  }
  ScoreBoardState getState() { return scoreBoardState; }
  UpdateStateResult updateState(const ScoreBoardState &newState)
  {
    UpdateStateResult result = UpdateAll(newState);
    if (result.success)
    {
      scoreBoardState = newState;
    }
    return result;
  }
  bool isReady() { return com->ConnectionStatus(); }

private:
  ScoreBoardState scoreBoardState;
  std::unique_ptr<ScoreBoardCom> com;
  UpdateStateResult UpdateAll(const ScoreBoardState &newState) const;
  bool sendCommandAndCheckResult(const String &command, UpdateStateResult& result) const;
};

#endif // SCOREBOARDSTATE_H