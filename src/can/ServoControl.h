#ifndef SERVO_CONTROL_H
#define SERVO_CONTROL_H

#define SERVO_TIMEOUT_MS 5000 // 3 sekundy

#include <Arduino.h>
#include <map>
#include <Preferences.h>
#include "ServoDevice.h"
#include "CanManager.h"
// #include "../network/PMPmanager.h"

const float P_MIN = -12.56f; // rad
const float P_MAX = 12.56f;
const float V_MIN = -33.0f; // rad/s
const float V_MAX = 33.0f;
const float T_MIN = -65.0f; // Ampery
const float T_MAX = 65.0f;
const float KP_MIN = 0.0f;
const float KP_MAX = 500.0f;
const float KD_MIN = 0.0f;
const float KD_MAX = 5.0f;

class ServoControl
{
private:
    std::map<uint8_t, ServoDevice> servos;
    CanManager &_can;
    Preferences _prefs;
    // PMPmanager &_slmp;

    TaskHandle_t _motionTaskHandle;
    static void _motionTask(void *pvParameters);

    enum ServoMode : byte
    {
        DutyCycle = 0,
        CurrentLoop = 1,
        BrakeCurrent = 2,
        VelocityLoop = 3,
        PositionLoop = 4,
        SetZero = 5,
        PositionVelocityLoop = 6,
        MITForceControl = 8,
        SERVO_OFF = 99
    };

    void _saveServoList();
    void _loadServoList();
    int float_to_uint(float x, float x_min, float x_max, int bits);

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
    bool parseCanTxFrame(CanFrame &frame, int id);
    void parsePMPFrameRx(int frame);
    void parsePMPFrameTx(int frame);

    void saveServoState(uint8_t id);
    bool deleteServo(uint8_t id);

    void checkServoConnection();

    std::function<void(const CanFrame &, bool)> onCanFrame;
};

#endif