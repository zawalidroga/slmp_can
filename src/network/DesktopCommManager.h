#ifndef DESKTOP_COMM_MANAGER_H
#define DESKTOP_COMM_MANAGER_H

#define STATUS_UPDATE_INTERVAL 5000 // ms

#include <Arduino.h>
#include <ArduinoJson.h>
#include <AsyncTCP.h>
#include "../can/ServoControl.h"
#include "../can/ServoDevice.h"

class DesktopCommManager
{
private:
    enum class CommandType
    {
        PING,
        STATUS,
        CAN_LOG,
        PMP_LOG,
        GET_PARAMS,
        SET_PARAMS,
        CONTROL,
        GET_SETTINGS,
        SET_SETTINGS,
        SERVOS_LIST

    };
    CommandType _resolveCommand(const String &cmd);

    ServoControl &_servos;

    void _handleStatus(AsyncClient *client);
    void _handleControl(JsonDocument &doc, AsyncClient *client);
    void _handlePing(JsonDocument &doc, AsyncClient *client);
    void _handleCanLog(JsonDocument &doc, AsyncClient *client);
    void _handlePMPLog(JsonDocument &doc, AsyncClient *client);
    void _handleGetParams(JsonDocument &doc, AsyncClient *client);
    void _handleSetParams(JsonDocument &doc, AsyncClient *client);
    void _handleSetSettings(JsonDocument &doc, AsyncClient *client);
    void _handleGetSettings(JsonDocument &doc, AsyncClient *client);

    IPAddress toIP(String ip_str);

public:
    DesktopCommManager(ServoControl &servoControl);
    void dataParser(uint8_t *data, size_t len, AsyncClient *client);
    void sendStatus(AsyncClient *client);

    void sendCanLog(const CanFrame &frame, bool isRx);
    void sendPMPLog(const String msg, bool isRx);

    void sendBroadcastStatus();
};

#endif