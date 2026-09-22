#ifndef SERVO_DEVICE_H
#define SERVO_DEVICE_H

#include <Arduino.h>
#include <Preferences.h>

enum class HomingState : uint8_t
{
    IDLE = 0,
    START_HOMING,
    MOVING_TO_SENSOR,
    STOPPING_ON_SENSOR,
    SENSOR_EXIT,
    STOPPING_OFF_SENSOR,
    SET_ZERO,
    SETTING_ZERO,
    COMPLETED,
    TIMEOUT_ERROR
};

class ServoDevice
{
private:
    int8_t _id;
    uint8_t _servoMode = 99;      // do cana do serwa
    uint8_t _positioningMode = 0; // esp
    int16_t _actualPosition = 0;
    int16_t _actualSpeed = 0;
    int16_t _actualCurrent = 0;
    int8_t _readErrorCode = 0;
    int8_t _actualTemperature = 0;
    int16_t _speed16 = (int16_t)speed;
    uint16_t _status;
    uint8_t _errorCode = 0;
    unsigned long _lastSeen = 0;
    unsigned long _startHomingTime;
    unsigned long _lastCommandtime = 0;
    unsigned long _lastBusyTime = 0;

    uint32_t _targetPosition = 0;
    uint16_t _inPositionOffset = 10; // domyślna wartość
    uint16_t _busyOffset = 50;

public:
    enum class ServoStatusFlags : uint16_t
    {
        NONE = 0,
        ENABLED = (1 << 0),
        READY_ON = (1 << 1),
        IN_POSITION = (1 << 2),
        HPR_COMPLETED = (1 << 3),
        HPR_REQUEST = (1 << 4),
        POSITIONING_COMPLETED = (1 << 5),
        BUSY = (1 << 6),             // od prędkości rzeczywistej
        BUSY_POSITIONING = (1 << 7), // od intencji ruchu - zadania target position
        HPR_BUSY = (1 << 8),
        EMPTY_3 = (1 << 9),
        EMPTY_4 = (1 << 10),
        EMPTY_5 = (1 << 11),
        EMPTY_6 = (1 << 12),
        ONLINE = (1 << 13),
        ERROR = (1 << 14),
        SERVO_ERR = (1 << 15)
    };

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

    int32_t target_position = 0;     // Docelowa pozycja z PLC (-360000000 do 360000000)
    int32_t target_speed = 0;        // Maksymalna prędkość z PLC w ERPM
    int16_t target_acceleration = 0; // Przyspieszenie z PLC (10 * elec RPM/s2)

    float current_p_deg = 0.0f;   // Bieżąca wirtualna pozycja w stopniach
    float current_v_deg_s = 0.0f; // Bieżąca wirtualna prędkość w stopniach/s

    int32_t current = 0;      // wartości od -60000 do 600000 co reprezentuje -60A - 60A
    int32_t brakeCurrent = 0; // wartości od 0 do 600000 co reprezentuje 0A - 60A
    int32_t speed = 0;        // wartości od -100000 do 100000 co reprezentuje -100000 do 100000 electrical RPM - dla position loop -32767 do 32676 bo przekazujemy int16
    int32_t position = 0;     // wartości od -360000000 do 360000000 co reprezentuje -36000 deg do 36000 deg
    int16_t acceleration = 0; // wartości od 0 do 32767 co reprezentuje 0 do 32767 *10 elec RPM/s2
    float position_rad;
    float speed_rad;
    float current_amp;
    float factorKP = 30;
    float factorKD = 1;
    int32_t homingSpeed = 100;
    int32_t homingTimeout = 30000; // ms

    bool servoInPosition = true;
    bool isOn = false;

    bool isHomingReverse = false;
    HomingState homingStep = HomingState::IDLE;
    bool isHomingSensorExitReverse = false;

    void makeItHome(bool sensorActive);

    void servoActualState();
    void readPrivateServoState();
    uint8_t getID() const;
    uint8_t getServoMode() const;
    uint8_t getPositioningMode() const;
    int16_t getParameters(RealParameter parName) const;

    int16_t getServoStatus() const;
    void setStatus(ServoStatusFlags flags);
    void clearStatus(ServoStatusFlags flags);
    bool isStatusSet(ServoStatusFlags flags) const;

    void setParameters(ParameterServo parName, uint32_t value);
    void setServoMonitor(RealParameter parName, int16_t value);
    void setServoMode(int8_t mode);
    void setZero();
    void setPositioningMode(uint8_t mode);

    void setPositionTarget(int32_t position);
    void setInPositionOffset(uint16_t offset);
    void setBusyPositioningOffset(uint16_t offset);

    void updateInPositionStatus();
    void updateBusyStatus();
    void updateServoStatus();

    void updateLastSeen();
    unsigned long getLastSeen() const;
    void updateLastCommandTime() { _lastCommandtime = millis(); };
    bool isCommandTimeout(unsigned long timeoutMs);

    void saveState(Preferences &prefs);
    void loadState(Preferences &prefs);

    void calculateNextStep(float dt);

    static RealParameter toRealParameter(int value);
    static ParameterServo toParameterServo(int value);

    std::function<void()> onStateChanged;

    ServoDevice(int8_t ID) : _id(ID), _status(0) {
                             };
};

#endif