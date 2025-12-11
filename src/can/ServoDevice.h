#ifndef SERVO_DEVICE_H
#define SERVO_DEVICE_H

#include <Arduino.h>
#include <Preferences.h>

class ServoDevice
{
private:
    int8_t _id;
    uint8_t _servoMode = 99;
    int16_t _actualPosition = 0;
    int16_t _actualSpeed = 0;
    int16_t _actualCurrent = 0;
    int8_t _readErrorCode = 0;
    int8_t _actualTemperature = 0;
    int16_t _speed16 = (int16_t)speed;

public:
    enum class RealParameter : uint8_t
    {
        SERVO_ID = 0,
        SERVO_MODE = 1,
        POSITION = 2,
        SPEED = 3,
        CURRENT = 4,
        ERROR = 5,
        TEMPERATURE = 6,

        INVALID_PARAM
    };

    enum class ParameterServo : uint8_t
    {
        CURRENT = 0,
        BRAKE_CURRENT = 1,
        SPEED = 2,
        POSITION = 3,
        ACCELERATION = 4,
        FACTOR_KP = 5,
        FACTOR_KD = 6,

        INVALID_PARAM
    };

    int32_t current = 0;      // wartości od -60000 do 600000 co reprezentuje -60A - 60A
    int32_t brakeCurrent = 0; // wartości od 0 do 600000 co reprezentuje 0A - 60A
    int32_t speed = 0;        // wartości od -100000 do 100000 co reprezentuje -100000 do 100000 electrical RPM - dla position loop -32767 do 32676 bo przekazujemy int16
    int32_t position = 0;     // wartości od -360000000 do 360000000 co reprezentuje -36000 deg do 36000 deg
    int16_t acceleration = 0; // wartości od 0 do 32767 co reprezentuje 0 do 32767 *10 elec RPM/s2
    int16_t factorKP = 0;
    int16_t factorKD = 0;

    bool servoInPosition = true;
    bool isOn = false;

    void servoActualState();
    void readPrivateServoState();
    uint8_t getID();
    uint8_t getServoMode();
    int16_t getAllParameters(RealParameter parName);
    int16_t getServoStatus();
    void setParameters(ParameterServo parName, uint32_t value);
    void setServoMonitor(RealParameter parName, uint32_t value);
    void setServoMode(int8_t mode);
    void setZero();

    void saveState(Preferences &prefs);
    void loadState(Preferences &prefs);

    static RealParameter toRealParameter(int value);
    static ParameterServo toParameterServo(int value);

    std::function<void()> onStateChanged;

    ServoDevice(int8_t ID)
    {
        _id = ID;
    };
};

#endif