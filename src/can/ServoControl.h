#ifndef SERVO_CONTROL_H
#define SERVO_CONTROL_H

#define SERVO_TIMEOUT_MS 3000 // 3 sekundy

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
    uint16_t idRecieved = 0;
    ServoControl(CanManager &can);

    void begin();
    void setServo(int8_t id, bool save = true);
    String listServoIDs();
    std::vector<uint8_t> getServoIds();
    std::vector<uint8_t> getOnlineServoIds();
    ServoDevice *getServo(int8_t id);
    const std::map<uint8_t, ServoDevice> &getServosMap() { return servos; }
    void parseCanRxFrame(const CanFrame &frame);
    void parseCanTxFrame(CanFrame &frame, int id);
    void parsePMPFrameRx(int frame);
    void parsePMPFrameTx(int frame);

    void saveServoState(uint8_t id);
    bool deleteServo(uint8_t id);

    void checkServoConnection();

    std::function<void(const CanFrame &, bool)> onCanFrame;
};

#endif