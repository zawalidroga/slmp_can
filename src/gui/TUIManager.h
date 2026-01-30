#ifndef TUI_MANAGER_H
#define TUI_MANAGER_H

#define STATUS_UPDATE_INTERVAL 5000 // ms

#include <AsyncTCP.h>
#include "../can/ServoControl.h"
#include "../can/ServoDevice.h"

enum class WindowState
{
    MAIN_MENU,
    SERVO_CONTROL,
    NETWORK_STAT,
    PPCAN_SETTINGS,
    SERVO_SETTINGS,
    SERVO_MONITOR,
    UR_MAMA_MONITOR,
    SERVO_GO,
    PARAM_SETTING,
    SERVO_AWAIT_ID,
    ADD_SERWO,
    DELETE_SERVO,
    NETWORK_MONITOR,
    TESTO,

};

class TUIManager
{
private:
    WindowState _currentState;
    AsyncClient *_activeClient = nullptr;
    int8_t _activeServo = 0;
    ServoControl &_servoControl;
    ServoDevice::ParameterServo _parameterToChange;
    WindowState _futureState;

    bool _isAllDigit(const String &str);

    void _printMainMenu();
    void _printServoControl();
    void _printServoMonitoring();
    void _printYourMamaMonitoring();
    void _printServoSettings();
    void _printSettings();
    void _printServoGO();
    void _printServoMonitor(int value, ServoDevice::RealParameter param);
    void _printServoSettingsMenu();

    void _mainMenuHandler(const String &c);
    void _servoControlHandler(const String &c);
    void _servoMonitorHandler();

public:
    bool isMonitoring = false;
    unsigned long lastStatusUpdate = 0;
    TUIManager(ServoControl &servoControl);
    void init(AsyncClient *client);
    void handleCommand(const String &command, ServoControl &servoControl);
    void onNewClientConnect(AsyncClient *client);
    void printNetworkMonitor(const String &msg);
    void printServoMonitor();
};

#endif