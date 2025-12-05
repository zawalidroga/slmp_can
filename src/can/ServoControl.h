#ifndef SERVO_CONTROL_H
#define SERVO_CONTROL_H

#include <Arduino.h>
#include <map>
#include "ServoDevice.h"
#include "CanManager.h"
#include "../network/SLMPmanager.h"

class ServoControl
{
private:
    std::map<uint8_t, ServoDevice> servos;
    CanManager &can;
    SLMPmanager &slmp;

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

    };

public:
    ServoControl(CanManager &can);
    void setServo(int8_t id);
    String listServoIDs();
    ServoDevice *getServo(int8_t id);
    void parseCanRxFrame(const CanFrame &frame);
    void parseCanTxFrame(CanFrame &frame, int id);
    void parseSLMPFrameRx(int frame);
    void parseSLMPFrameTx(int frame);
};

#endif