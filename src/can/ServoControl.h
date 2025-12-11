#ifndef SERVO_CONTROL_H
#define SERVO_CONTROL_H

#include <Arduino.h>
#include <map>
#include <Preferences.h>
#include "ServoDevice.h"
#include "CanManager.h"
// #include "../network/PMPmanager.h"

class ServoControl
{
private:
    std::map<uint8_t, ServoDevice> servos;
    CanManager &_can;
    Preferences _prefs;
    // PMPmanager &_slmp;

    enum ServoMode : byte
    {
        DutyCycle = 0,
        CurrentLoop = 1,
        BrakeCurrent = 2,
        VelocityLoop = 3,
        PositionLoop = 4,
        SetZero = 5,
        PositionVelocityLoop = 6,
        MITVelocityLoop = 7,
        SERVO_OFF = 99

    };

    void _saveServoList();
    void _loadServoList();

public:
    ServoControl(CanManager &can);

    void begin();
    void setServo(int8_t id, bool save = true);
    String listServoIDs();
    ServoDevice *getServo(int8_t id);
    void parseCanRxFrame(const CanFrame &frame);
    void parseCanTxFrame(CanFrame &frame, int id);
    void parseSLMPFrameRx(int frame);
    void parseSLMPFrameTx(int frame);

    void saveServoState(uint8_t id);
};

#endif