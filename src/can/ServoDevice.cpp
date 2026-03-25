#include "ServoDevice.h"
#include "../network/NetworkManager.h"

void ServoDevice::setServoMode(int8_t mode)
{

    if (mode >= 0 && mode <= 7)
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
uint8_t ServoDevice::getID() const
{
    return _id;
};

uint8_t ServoDevice::getServoMode() const
{
    return _servoMode;
};
uint8_t ServoDevice::getPositioningMode() const
{
    return _positioningMode;
};

void ServoDevice::setPositioningMode(uint8_t mode)
{
    _positioningMode = mode;
}

int16_t ServoDevice::getParameters(RealParameter parName) const
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

// ###################### STATUSY SERWA ##########################

int16_t ServoDevice::getServoStatus() const
{
    return _status;
};
void ServoDevice::setStatus(ServoStatusFlags flags)
{
    _status |= static_cast<uint16_t>(flags);
};
void ServoDevice::clearStatus(ServoStatusFlags flags)
{
    _status &= ~static_cast<uint16_t>(flags);
};
bool ServoDevice::isStatusSet(ServoStatusFlags flags) const
{
    return (_status & static_cast<uint16_t>(flags)) != 0;
};

void ServoDevice::updateInPositionStatus()
{
    if (abs((int)_targetPosition - (int)_actualPosition) <= _inPositionOffset)
    {
        if (!isStatusSet(ServoStatusFlags::IN_POSITION))
        {
            setStatus(ServoStatusFlags::IN_POSITION);
            setStatus(ServoStatusFlags::POSITIONING_COMPLETED);
            clearStatus(ServoStatusFlags::BUSY_POSITIONING);
        }
    }
    else
    {
        clearStatus(ServoStatusFlags::IN_POSITION);
        clearStatus(ServoStatusFlags::POSITIONING_COMPLETED);
    }
};

void ServoDevice::updateBusyStatus()
{
    if (abs((int)_actualSpeed) > _busyOffset)
    {
        setStatus(ServoStatusFlags::BUSY);
        if (_servoMode == 6)
        {
            setStatus(ServoStatusFlags::BUSY_POSITIONING);
        }
    }
    else
    {
        clearStatus(ServoStatusFlags::BUSY);
        clearStatus(ServoStatusFlags::BUSY_POSITIONING);
    };
};

void ServoDevice::updateServoStatus()
{
}

// ##############################################################

// ##################### servo monitory i settery #######################
void ServoDevice::setServoMonitor(RealParameter param, int16_t value)
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

void ServoDevice::setPositionTarget(int32_t position)
{
    _targetPosition = position;
    clearStatus(ServoStatusFlags::IN_POSITION);
    clearStatus(ServoStatusFlags::POSITIONING_COMPLETED);
    setStatus(ServoStatusFlags::BUSY_POSITIONING);
};

void ServoDevice::setInPositionOffset(uint16_t offset)
{
    _inPositionOffset = offset;
};

void ServoDevice::setBusyPositioningOffset(uint16_t offset)
{
    _busyOffset = offset;
};

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

// ######################### IS SERVO LIVE CHECK ########################
void ServoDevice::updateLastSeen()
{
    _lastSeen = millis();
};

unsigned long ServoDevice::getLastSeen() const
{
    return _lastSeen;
};

bool ServoDevice::isCommandTimeout(unsigned long timeoutMs)
{
    if (_lastCommandtime == 0 || !isStatusSet(ServoStatusFlags::ENABLED))
        return false;
    return (millis() - _lastCommandtime > timeoutMs);
};

// ######################## BAZOWANIE ########################

void ServoDevice::makeItHome(bool sensorAcitve)
{

    switch (homingStep)
    {
    case HomingState::IDLE:
        break;
    case HomingState::START_HOMING:
        NetworkManager::getInstance().sendSystemLog("[SERVO] START HOMING");
        _startHomingTime = millis();
        this->setStatus(ServoDevice::ServoStatusFlags::HPR_BUSY);
        this->setServoMode(3); // Veloxity loop
        this->speed = isHomingReverse ? -this->homingSpeed : this->homingSpeed;
        homingStep = HomingState::MOVING_TO_SENSOR;
        break;

    case HomingState::MOVING_TO_SENSOR:

        if (sensorAcitve)
        {
            this->speed = 0;
            homingStep = HomingState::SENSOR_EXIT;
            NetworkManager::getInstance().sendSystemLog("[SERVO] SENSOR FOUND");
        }
        else if (millis() - _startHomingTime > homingTimeout)
        {
            homingStep = HomingState::TIMEOUT_ERROR;
        }
        break;
    case HomingState::SENSOR_EXIT:

        if (sensorAcitve)
        {
            this->speed = isHomingReverse ? 500 : -500;
        }
        else
        {
            NetworkManager::getInstance().sendSystemLog("[SERVO] SENSOR EXIT");
            this->speed = 0;
            homingStep = HomingState::SET_ZERO;
        }
        break;
    case HomingState::SET_ZERO:
        this->setServoMode(5);
        homingStep = HomingState::SETTING_ZERO;
        NetworkManager::getInstance().sendSystemLog("[SERVO] SETTING ZERO");
        break;

    case HomingState::SETTING_ZERO:

        if (this->_actualPosition == 0)
        {
            NetworkManager::getInstance().sendSystemLog("[SERVO] POSITION SET TO 0");
            setStatus(ServoStatusFlags::HPR_COMPLETED);
            clearStatus(ServoStatusFlags::HPR_REQUEST);
            homingStep = HomingState::COMPLETED;
        }
        break;
    case HomingState::COMPLETED:
        NetworkManager::getInstance().sendSystemLog("[SERVO] HOMING COMPLETED");
        this->position = this->getParameters(ServoDevice::RealParameter::POSITION) * 10000;
        this->setServoMode(6);
        homingStep = HomingState::IDLE;
        this->clearStatus(ServoDevice::ServoStatusFlags::HPR_BUSY);
        break;
    case HomingState::TIMEOUT_ERROR:
        setStatus(ServoStatusFlags::ERROR);
        this->_errorCode = 1;

        break;
    };
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