#ifndef TIMESERVER_HPP
#define TIMESERVER_HPP
#include "Arduino.h"
#include "time.h"

class timeServer
{
public:
    static void getTimefromServer();
    static tm getTimeStruct();
private:
    tm timeInfo;

};

#endif