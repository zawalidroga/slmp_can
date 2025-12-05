#include "ServoDevice.h";

void ServoDevice::setServoMode(int8_t mode)
{
    if (7 >= _servoMode >= 0)
    {
        _servoMode = mode;
    };
};

void ServoDevice::servoActualState() {};
void ServoDevice::readPrivateServoState() {};
uint8_t ServoDevice::getID()
{
    return _id;
};

uint8_t ServoDevice::getServoMode()
{
    return _servoMode;
};

int16_t ServoDevice::getAllParameters(RealParameter parName)
{
    switch (parName)
    {
    case RealParameter::SERVO_ID:
        break;
    case RealParameter::SERVO_MODE:
        break;
    }
};
void ServoDevice::setParameters(ParameterServo parName, uint32_t value)
{
    switch (parName)
    {
    case ParameterServo::CURRENT:
        current = value;
        break;
    case ParameterServo::BRAKE_CURRENT:
        brakeCurrent = value;
        break;
    case ParameterServo::SPEED:
        speed = value;
        break;
    case ParameterServo::POSITION:
        position = value;
        break;
    case ParameterServo::ACCELERATION:
        acceleration = value;
        break;
    case ParameterServo::FACTOR_KP:
        factorKP = value;
        break;
    case ParameterServo::FACTOR_KD:
        factorKD = value;
        break;
    }
};

void ServoDevice::setZero() {};

// ######################## FUNKCJE POMOCNICZE ########################

ServoDevice::RealParameter ServoDevice::toRealParameter(int value)
{
    if (value >= static_cast<int>(RealParameter::SERVO_ID) && value < static_cast<int>(RealParameter::INVALID_PARAM))
    {
        return static_cast<RealParameter>(value);
    }
    return RealParameter::INVALID_PARAM;
};

ServoDevice::ParameterServo ServoDevice::toParameterServo(int value)
{
    if (value >= static_cast<int>(ParameterServo::CURRENT) && value < static_cast<int>(RealParameter::INVALID_PARAM))
    {
        return static_cast<ParameterServo>(value);
    }
    return ParameterServo::INVALID_PARAM;
};