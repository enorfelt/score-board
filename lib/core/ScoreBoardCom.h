#ifndef COMPORTCLASS_H
#define COMPORTCLASS_H

#include <SoftwareSerial.h>

class ScoreBoardCom {
  public:
    ScoreBoardCom(int rx, int tx);
    bool comportsexists();
    bool Open();
    void Close();
    bool IsOpen();
    bool SendCommandLookForString(String command, String stringToLookFor);
    String read_ee(int addr);
    String read_int(String command);
    String fetchedOutputString;

  private:
    int fetchedOutputInt;
    byte bEEPbyte;
    String debugBuffer;
    SoftwareSerial boardSerial;
    unsigned long timeout;
};

#endif
