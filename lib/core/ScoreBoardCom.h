#ifndef COMPORTCLASS_H
#define COMPORTCLASS_H

#include <SoftwareSerial.h>
#include <string>

class ScoreBoardCom {
  public:
    ScoreBoardCom(const int rx, const int tx);
    bool Open();
    void Close();
    bool IsOpen();
    bool ConnectionStatus();
    bool SendCommandLookForString(const char* command, const char* stringToLookFor);
    std::string fetchedOutputString;

  private:
    SoftwareSerial boardSerial;
    unsigned long timeout;
};

#endif
