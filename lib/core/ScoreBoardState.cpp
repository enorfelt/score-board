#include <ScoreBoardState.h>

const char *MESSAGE_PASS = "Ok";

void ScoreBoardStateStore::begin()
{
  com->Open();
  delay(200); // Allow SoftwareSerial and board to settle before sending
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

  char command[20];

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

  // Out LEDs: use modulo 3 so the display cycles correctly across both halves of an inning.
  // 0,3 → both off | 1,4 → Out1 on | 2,5 → Out1+Out2 on
  int outsDisplay = newState.outsInInning % 3;
  if (!sendCommandAndCheckResult(outsDisplay >= 1 ? "wb 5 0 1" : "wb 5 0 0", result))
  {
    return result;
  }

  // Out 2 LED: lit when at least 2 outs in the current half-inning
  if (!sendCommandAndCheckResult(outsDisplay >= 2 ? "wb 5 1 1" : "wb 5 1 0", result))
  {
    return result;
  }


  return result;
}