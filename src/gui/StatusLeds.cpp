#include "StatusLeds.h"

StatusLEDs::StatusLEDs() : _heartbeatState(true) {
                           };

void StatusLEDs::begin()
{
    pinMode(RED_PIN, OUTPUT);
    pinMode(ORANGE_PIN, OUTPUT);
    pinMode(GREEN_PIN, OUTPUT);

    digitalWrite(RED_PIN, HIGH);
    digitalWrite(ORANGE_PIN, HIGH);
    digitalWrite(GREEN_PIN, HIGH);
    delay(500);
    digitalWrite(RED_PIN, LOW);
    digitalWrite(ORANGE_PIN, LOW);
    digitalWrite(GREEN_PIN, LOW);
};

void StatusLEDs::update()
{
    _processGreen();
    _processOrange();
    _processRed();
};

void StatusLEDs::notifyCanActivity()
{
    digitalWrite(ORANGE_PIN, HIGH);
    _orangeOffTime = millis() + 40;
};

void StatusLEDs::setError(bool active)
{
    _erroAvtive = active;
};

void StatusLEDs::setPMPError(bool active)
{
    _pmpError = active;
};

void StatusLEDs::notifyPMPActivity()
{
    digitalWrite(GREEN_PIN, LOW);
    _greenOnTime = millis() + 40;
};

void StatusLEDs::_processGreen()
{
    if (_heartbeatState)
    {
        bool state = (millis() / 100) % 2;
        digitalWrite(GREEN_PIN, state);
    }
    else if (!_heartbeatState && millis() > _greenOnTime)
    {
        digitalWrite(GREEN_PIN, HIGH);
        _greenOnTime = 0;
    }
};

void StatusLEDs::_processOrange()
{
    if (_orangeOffTime > 0 && millis() > _orangeOffTime)
    {
        digitalWrite(ORANGE_PIN, LOW);
        _orangeOffTime = 0;
    }
};

void StatusLEDs::_processRed()
{
    if (_erroAvtive)
    {
        bool state = (millis() / 100) % 2;
        digitalWrite(RED_PIN, state);
    }
    else
    {
        digitalWrite(RED_PIN, LOW);
    }
};