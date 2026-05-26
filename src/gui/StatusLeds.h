#ifndef STATUS_LEDS
#define STATUS_LEDS

#include <Arduino.h>

class StatusLEDs
{
private:
    unsigned long _lastUpdate = 0;
    unsigned long _orangeOffTime = 0;
    unsigned long _greenOnTime = 0;

    bool _heartbeatState;
    bool _erroAvtive = false;
    bool _pmpError = false;

    void _processGreen();
    void _processOrange();
    void _processRed();

public:
    static const uint8_t RED_PIN = 14;
    static const uint8_t ORANGE_PIN = 12;
    static const uint8_t GREEN_PIN = 15;

    StatusLEDs();
    void begin();
    void update();

    void rstHeartbeat() { _heartbeatState = false; };
    void notifyCanActivity();
    void setError(bool active);
    void setPMPError(bool active);
    void notifyPMPActivity();
};

#endif // STATUS_LEDS