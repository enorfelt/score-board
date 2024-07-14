#include <ScoreBoardState.h>

const char *MESSAGE_PASS = "Ok";

void ScoreBoardStateStore::begin() {
  com->Open();
  this->_isReady = com->ConnectionStatus();
}

bool ScoreBoardStateStore::sendCommandAndCheckResult(const String& command, UpdateStateResult& result) const {
  if (!com->SendCommandLookForString(command, MESSAGE_PASS)) {
    result.success = false;
    result.message = "Unexpected answer from unit. Command was " + command + ". Was looking for string " + MESSAGE_PASS + " in output " + com->fetchedOutputString;
    return false;
  }
  return true;
}

UpdateStateResult ScoreBoardStateStore::UpdateAll(const ScoreBoardState& newState) const {
  // Your implementation here
  UpdateStateResult result;
  result.success = true;

  String command;

  // Home
  int homeScoreTens = newState.home / 10;
  command = "wd 0 " + String(homeScoreTens);
  if (!sendCommandAndCheckResult(command, result)) {
    return result;
  }

  command = "wd 1 " + String(newState.home - (10 * homeScoreTens));
  if (!sendCommandAndCheckResult(command, result)) {
    return result;
  }

  // Innings
  command = "wd 2 " + String(newState.inning);
  if (!sendCommandAndCheckResult(command, result)) {
    return result;
  }

  // Guest
  int awayScoreTens = newState.away / 10;
  command = "wd 3 " + String(awayScoreTens);
  if (!sendCommandAndCheckResult(command, result)) {
    return result;
  }

  command = "wd 4 " + String(newState.away - (10 * awayScoreTens));
  if (!sendCommandAndCheckResult(command, result)) {
    return result;
  }

  // Out
  if ((newState.outsInInning > 0 && newState.outsInInning < 3) || newState.outsInInning > 3) {
    if (!sendCommandAndCheckResult("wb 5 0 1", result)) {
      return result;
    }
  } else {
    if (!sendCommandAndCheckResult("wb 5 0 0", result)) {
      return result;
    }
  }

  if ((newState.outsInInning > 1 && newState.outsInInning < 3) || newState.outsInInning > 4) {
    if (!sendCommandAndCheckResult("wb 5 1 1", result)) {
      return result;
    }
  } else {
    if (!sendCommandAndCheckResult("wb 5 1 0", result)) {
      return result;
    }
  }

  return result;
}