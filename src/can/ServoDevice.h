#ifndef SERVO_DEVICE_H
#define SERVO_DEVICE_H

#include <Arduino.h>

class ServoDevice
{
private:
    int8_t _id;
    uint8_t _servoMode;
    int16_t _actualPosition;
    int16_t _actualSpeed;
    int16_t _actualCurrent;
    int8_t _readErrorCode;
    int8_t _actualTemperature;
    int16_t _speed16 = (int16_t)speed;

public:
    int32_t current = 0;      // wartości od -60000 do 600000 co reprezentuje -60A - 60A
    int32_t brakeCurrent = 0; // wartości od 0 do 600000 co reprezentuje 0A - 60A
    int32_t speed = 0;        // wartości od -100000 do 100000 co reprezentuje -100000 do 100000 electrical RPM - dla position loop -32767 do 32676 bo przekazujemy int16
    int32_t position = 0;     // wartości od -360000000 do 360000000 co reprezentuje -36000 deg do 36000 deg
    int16_t acceleration = 0; // wartości od 0 do 32767 co reprezentuje 0 do 32767 *10 elec RPM/s2
    int16_t factorKP = 0;
    int16_t factorKD = 0;
    int16_t speed16 = (int16_t)speed;
    bool servoInPosition = true;
    bool isOn = false;

    void servoActualState();
    void readPrivateServoState();
    uint8_t getID();
    uint8_t getServoMode();
    int16_t getActualPosition();
    int16_t getActualSpeed();
    int16_t getActualCurrent();
    int8_t getReadErrorCode();
    int8_t getActualTemperature();
    int16_t getAllParameters(uint16_t parIndex);
    void setParameters(uint16_t parIndex, uint32_t value);

    ServoDevice(int8_t ID)
    {
        _id = ID;
    };
};

#endif