#include <ScoreBoardState.h>

const char *MESSAGE_PASS = "Ok";

void ScoreBoardStateStore::begin()
{
  com->Open();
  this->_isReady = com->ConnectionStatus();
}

bool ScoreBoardStateStore::sendCommandAndCheckResult(const char *command, UpdateStateResult &result) const
{
  if (!com->SendCommandLookForString(command, MESSAGE_PASS))
  {
    result.success = false;
    result.message = "Unexpected answer from unit. Command was ";
    result.message += command;
    result.message += ". Was looking for string ";
    result.message += MESSAGE_PASS;
    result.message += " in output ";
    result.message += com->fetchedOutputString;
    return false;
  }
  return true;
}

UpdateStateResult ScoreBoardStateStore::UpdateAll(const ScoreBoardState &newState) const
{
  // Your implementation here
  UpdateStateResult result;
  result.success = true;

  char command[11];

  // Home
  int homeScoreTens = newState.home / 10;
  snprintf(command, sizeof(command), "wd 0 %d", homeScoreTens);
  if (!sendCommandAndCheckResult(command, result))
  {
    return result;
  }

  snprintf(command, sizeof(command), "wd 1 %d", newState.home - (10 * homeScoreTens));
  if (!sendCommandAndCheckResult(command, result))
  {
    return result;
  }

  // Innings
  snprintf(command, sizeof(command), "wd 2 %d", newState.inning);
  if (!sendCommandAndCheckResult(command, result))
  {
    return result;
  }

  // Guest
  int awayScoreTens = newState.away / 10;
  snprintf(command, sizeof(command), "wd 3 %d", awayScoreTens);
  if (!sendCommandAndCheckResult(command, result))
  {
    return result;
  }

  snprintf(command, sizeof(command), "wd 4 %d", newState.away - (10 * awayScoreTens));
  if (!sendCommandAndCheckResult(command, result))
  {
    return result;
  }

  // Out 1 LED: lit when at least 1 out
  if (!sendCommandAndCheckResult(newState.outsInInning >= 1 ? "wb 5 0 1" : "wb 5 0 0", result))
  {
    return result;
  }

  // Out 2 LED: lit when at least 2 outs
  if (!sendCommandAndCheckResult(newState.outsInInning >= 2 ? "wb 5 1 1" : "wb 5 1 0", result))
  {
    return result;
  }


  return result;
}