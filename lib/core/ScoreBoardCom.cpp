#include "ScoreBoardCom.h"
// 2.	Timeouts: The SendCommandLookForString(), read_ee(), and read_int() methods use a fixed delay of 100ms to wait for a response after sending a command. This might not be enough time for some devices, especially if they're busy or have a slow baud rate. You might consider adding a timeout parameter to these methods, or using a more sophisticated method to wait for a response.
// 3.	Buffer Overflow: The read_ee() and read_int() methods read data into a String until there's no more data available. If the device sends a lot of data, this could potentially cause a buffer overflow. You might consider adding a maximum length to the String, or using a fixed-size buffer and reading data in chunks.
// 4.	Command Parsing: The SendCommandLookForString(), read_ee(), and read_int() methods send a command and then look for a specific string in the response. This is a simple and effective way to parse the response, but it might not work well if the device's responses are not consistent, or if they contain the string you're looking for as part of another command or piece of data. You might consider implementing a more robust command parsing system, possibly using regular expressions or a state machine.
// 5.	Non-Blocking Code: The SendCommandLookForString(), read_ee(), and read_int() methods are blocking, meaning they stop the rest of your program from running while they're waiting for a response. This might not be a problem in a simple program, but in a more complex program it could cause responsiveness issues. You might consider rewriting these methods to be non-blocking, possibly using callbacks or a state machine.

ScoreBoardCom::ScoreBoardCom(int rx, int tx) : boardSerial(rx, tx)
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
  if (SendCommandLookForString("C", "Control SW"))
  {
    return true;
  }

  if (SendCommandLookForString("version", "Control SW"))
  {
    return true;
  }

  return false;
}

bool ScoreBoardCom::SendCommandLookForString(const String &command, const String &stringToLookFor)
{
  fetchedOutputString = "";
  Serial.println("Sending command: " + command);
  boardSerial.println(command);
  unsigned long startTime = millis();
  while (millis() - startTime < timeout)
  {
    // yield();
    Serial.println("Waiting for response...");
    while (boardSerial.available()) {
      fetchedOutputString += (char)boardSerial.read();
      Serial.println("Response so far: " + fetchedOutputString);
    }

    if (fetchedOutputString.indexOf(stringToLookFor) != -1)
    {
      Serial.println("Ouput string: " + fetchedOutputString);
      return true;
    }
    delay(50);
    ESP.wdtFeed();
  }
  return false;
}

String ScoreBoardCom::read_ee(int addr)
{
  String fetchedOutputString = "";
  boardSerial.println("read_ee " + String(addr));
  unsigned long startTime = millis();
  while (millis() - startTime < timeout)
  {
    while (boardSerial.available())
    {
      fetchedOutputString += (char)boardSerial.read();
    }
  }
  return fetchedOutputString;
}

String ScoreBoardCom::read_int(String command)
{
  String fetchedOutputString = "";
  boardSerial.println(command);
  unsigned long startTime = millis();
  while (millis() - startTime < timeout)
  {
    while (boardSerial.available())
    {
      fetchedOutputString += (char)boardSerial.read();
    }
  }
  return fetchedOutputString;
}
