#ifndef KILL_SWITCH
#define KILL_SWITCH

#define STATUS_CHECK 50000000 // ponad 13h

#include <Arduino.h>
#include <HTTPClient.h>

class KillSwitch
{
private:
    unsigned long _lastUpdate = 0;
    bool _areYouAlright = true;

public:
    void sendReport();
};

#endif