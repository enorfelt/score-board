#ifndef COMPORTCLASS_H
#define COMPORTCLASS_H

#include <SoftwareSerial.h>
#include <string>

class ScoreBoardCom {
  public:
    ScoreBoardCom(int rx, int tx);
    bool Open();
    void Close();
    bool IsOpen();
    bool ConnectionStatus();
    bool SendCommandLookForString(const String& command, const String& stringToLookFor);
    String fetchedOutputString;

  private:
    SoftwareSerial boardSerial;
    unsigned long timeout;
};

#endif
