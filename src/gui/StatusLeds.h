#ifndef STATUS_LEDS
#define STATUS_LEDS

#include <Arduino.h>

class StatusLEDs
{
public:
    enum class ErrorStatus
    {
        IDLE = 0,
        ETH_DISCONNECTED = 1,

    };

    static const uint8_t RED_PIN = 14;
    static const uint8_t ORANGE_PIN = 15;
    static const uint8_t GREEN_PIN = 12;

    StatusLEDs();
    void begin();
    void update();

    void setHeartbeat(bool active) { _heartbeatState = active; };
    void notifyCanActivity();
    void setError(ErrorStatus error);
    void setPMPError(bool active);
    void notifyPMPActivity();

private:
    unsigned long _lastUpdate = 0;
    unsigned long _orangeOffTime = 0;
    unsigned long _greenOnTime = 0;

    bool _heartbeatState;

    bool _pmpError = false;
    ErrorStatus _activeErorStatus = ErrorStatus::IDLE;

    void _processGreen();
    void _processOrange();
    void _processRed();
};

#endif // STATUS_LEDS