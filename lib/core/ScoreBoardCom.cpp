#include "ScoreBoardCom.h"

ScoreBoardCom::ScoreBoardCom(const int rx, const int tx) : boardSerial(rx, tx)
{
  timeout = 3000; // 1 second timeout
}

bool ScoreBoardCom::Open()
{
  if (!boardSerial.isListening())
  {
    boardSerial.begin(9600);
    return true;
  }
  else
  {
    return false;
  }
}

void ScoreBoardCom::Close()
{
  boardSerial.end();
}

bool ScoreBoardCom::IsOpen()
{
  return boardSerial.isListening();
}

bool ScoreBoardCom::ConnectionStatus()
{
  // Send "C\n" — matches what the original Windows GUI sends (WriteLine("C")).
  // The board enters command mode and responds with "Control SW".
  if (SendCommandLookForString("C", "Control SW"))
    return true;

  // Fallback: board may already be in command mode
  return SendCommandLookForString("version", "Control SW");
}


bool ScoreBoardCom::SendCommandLookForString(const char *command, const char *stringToLookFor, bool withNewline)
{
  Serial.print(F("Sending command: "));
  Serial.println(command);

  // Drain any stale bytes before sending
  while (boardSerial.available()) boardSerial.read();

  size_t written;
  if (withNewline)
  {
    // Send command + '\r\n' — the board responds to '\r' (screen terminal) and '\n'
    // (C# GUI via WriteLine). Sending both maximises compatibility.
    written = boardSerial.print(command);
    written += boardSerial.print('\r');
    written += boardSerial.print('\n');
  }
  else
    written = boardSerial.print(command);

  Serial.printf("Wrote %u bytes. Waiting for response...\n", (unsigned)written);

  const unsigned long startTime = millis();

  fetchedOutputString.clear();
  while (millis() - startTime < timeout)
  {
    while (boardSerial.available())
    {
      char c = boardSerial.read();
      fetchedOutputString += c;
      if (fetchedOutputString.size() > 512)
        break;
    }

    if (fetchedOutputString.find(stringToLookFor) != std::string::npos)
    {
      Serial.print(F("Response: "));
      Serial.println(fetchedOutputString.c_str());
      return true;
    }
#ifdef ARDUINO_ARCH_ESP8266
    ESP.wdtFeed(); // feed watchdog — do NOT call yield() here: ESPAsyncWebServer
                   // callbacks run in SYS (interrupt) context where yield() panics
#endif
  }
  Serial.print(F("Timeout. Received: '"));
  Serial.print(fetchedOutputString.empty() ? "(nothing)" : fetchedOutputString.c_str());
  Serial.println(F("'"));
  return false;
}