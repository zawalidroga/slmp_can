#include "ServoDevice.h"

void ServoDevice::setServoMode(int8_t mode)
{

    if (7 >= _servoMode >= 0)
    {
        _servoMode = mode;
        if (onStateChanged)
        {
            onStateChanged();
        };
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
        return (int16_t)_id;
        break;
    case RealParameter::SERVO_MODE:
        return _servoMode;
        break;
    case RealParameter::POSITION:
        return _actualPosition;
        break;
    case RealParameter::SPEED:
        return _actualSpeed;
        break;
    case RealParameter::CURRENT:
        return _actualCurrent;
        break;
    case RealParameter::ERROR:
        return _readErrorCode;
        break;
    case RealParameter::TEMPERATURE:
        return _actualTemperature;
        break;
    };
    return 0;
};

int16_t ServoDevice::getServoStatus()
{
    return 3;
};

void ServoDevice::setServoMonitor(RealParameter param, uint32_t value)
{
    switch (param)
    {
    case RealParameter::SERVO_ID:
        _id = value;
        break;
    case RealParameter::SERVO_MODE:
        _servoMode = value;
        break;
    case RealParameter::SPEED:
        _actualSpeed = value;
        break;
    case RealParameter::POSITION:
        _actualPosition = value;
        break;
    case RealParameter::CURRENT:
        _actualCurrent = value;
        break;
    case RealParameter::ERROR:
        _readErrorCode = value;
        break;
    case RealParameter::TEMPERATURE:
        _actualTemperature = value;
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
    if (onStateChanged)
    {
        onStateChanged();
    };
};

void ServoDevice::setZero() {};

// ######################### ZAPIS PARAMETRÓW ######################

void ServoDevice::saveState(Preferences &prefs)
{

    prefs.putUChar("mode", _servoMode);
    prefs.putInt("current", current);
    prefs.putInt("brakeCurr", brakeCurrent);
    prefs.putInt("speed", speed);
    prefs.putInt("position", position);
    prefs.putShort("accel", acceleration);
    prefs.putShort("kp", factorKP);
    prefs.putShort("kd", factorKD);

    Serial.printf("[NVS] Zapisano stan dla serwa ID: %d\n", _id);
};

void ServoDevice::loadState(Preferences &prefs)
{

    _servoMode = prefs.getUChar("mode", 0);
    current = prefs.getInt("current", 0);
    brakeCurrent = prefs.getInt("brakeCurr", 0);
    speed = prefs.getInt("speed", 0);
    position = prefs.getInt("position", 0);
    acceleration = prefs.getShort("accel", 0);
    factorKP = prefs.getShort("kp", 0);
    factorKD = prefs.getShort("kd", 0);

    Serial.printf("[NVS] Wczytano stan dla serwa ID: %d\n", _id);
};

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