
#include "DesktopCommManager.h"
#include "NetworkManager.h"
#include "../Settings/SettingManager.h"

DesktopCommManager::DesktopCommManager(ServoControl &servoControl) : _servos(servoControl)
{
}

void DesktopCommManager::dataParser(uint8_t *data, size_t len, AsyncClient *client)
{
    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, data, len);

    if (err)
    {
        Serial.print(F("[JSON] [??] deserialize failed: "));
        NetworkManager::getInstance().sendSystemLog(err.f_str());
        Serial.println(err.f_str());
        return;
    };

    switch (_resolveCommand(doc["cmd"] | ""))
    {
    case CommandType::STATUS:
        _handleStatus(client);

        break;
    case CommandType::CAN_LOG:
        break;
    case CommandType::PMP_LOG:
        break;
    case CommandType::GET_PARAMS:
        break;
    case CommandType::GET_SETTINGS:
        break;
    case CommandType::SET_PARAMS:
        break;
    case CommandType::SET_SETTINGS:

        SettingMenager::getInstance().setIPAdrdess(toIP(doc["ip_address"]));
        SettingMenager::getInstance().setGateway(toIP(doc["gateway"]));
        SettingMenager::getInstance().setSubnet(toIP(doc["subnet"]));
        SettingMenager::getInstance().setPrimaryDNS(toIP(doc["primary_dns"]));
        SettingMenager::getInstance().setSecondaryDNS(toIP(doc["secondary_dns"]));
        SettingMenager::getInstance().setTCPPort(doc["tcp_port"]);
        SettingMenager::getInstance().setUDPPort(doc["udp_port"]);
        SettingMenager::getInstance().save();
        break;
    case CommandType::CONTROL:
        break;
    case CommandType::SERVOS_LIST:
        if (doc["subcmd"] == "add")
        {
            _servos.setServo(doc["id"]);
            NetworkManager::getInstance().sendSystemLog(doc["id"]);
        }
        else if (doc["subcmd"] == "delete")
        {
            _servos.deleteServo(doc["id"]);
            NetworkManager::getInstance().sendSystemLog(doc["id"]);
        }
        break;
    case CommandType::PING:
        this->_handlePing(doc, client);
        break;
    default:
        break;
    }
};

void DesktopCommManager::sendStatus(AsyncClient *client) {

};

DesktopCommManager::CommandType DesktopCommManager::_resolveCommand(const String &cmd)
{
    if (cmd == "status")
        return CommandType::STATUS;
    if (cmd == "get_params")
        return CommandType::GET_PARAMS;
    if (cmd == "set_params")
        return CommandType::SET_PARAMS;
    if (cmd == "can_log")
        return CommandType::CAN_LOG;
    if (cmd == "pmp_log")
        return CommandType::PMP_LOG;
    if (cmd == "control")
        return CommandType::CONTROL;
    if (cmd == "get_settings")
        return CommandType::GET_SETTINGS;
    if (cmd == "set_settings")
        return CommandType::SET_SETTINGS;
    if (cmd == "ping")
        return CommandType::PING;
    if (cmd == "servos_list")
        return CommandType::SERVOS_LIST;
}

void DesktopCommManager::_handleStatus(AsyncClient *client)
{
    sendBroadcastStatus();
}

void DesktopCommManager::_handlePing(JsonDocument &doc, AsyncClient *client)
{
    JsonDocument response;

    response["type"] = "pong";

    String output;
    serializeJson(response, output);
    output += "\n";
    client->write(output.c_str(), output.length());
};

void DesktopCommManager::_handleControl(JsonDocument &doc, AsyncClient *client)
{
    JsonDocument response;

    String output;
    serializeJson(response, output);
    output += "\n";
    client->write(output.c_str(), output.length());
};
void DesktopCommManager::_handleCanLog(JsonDocument &doc, AsyncClient *client)
{
    JsonDocument response;

    String output;
    serializeJson(response, output);
    output += "\n";
    client->write(output.c_str(), output.length());
};
void DesktopCommManager::_handlePMPLog(JsonDocument &doc, AsyncClient *client)
{
    JsonDocument response;

    String output;
    serializeJson(response, output);
    output += "\n";
    client->write(output.c_str(), output.length());
};
void DesktopCommManager::_handleGetParams(JsonDocument &doc, AsyncClient *client)
{
    JsonDocument response;

    String output;
    serializeJson(response, output);
    output += "\n";
    client->write(output.c_str(), output.length());
};
void DesktopCommManager::_handleSetParams(JsonDocument &doc, AsyncClient *client)
{
    JsonDocument response;

    String output;
    serializeJson(response, output);
    output += "\n";
    client->write(output.c_str(), output.length());
};
void DesktopCommManager::_handleSetSettings(JsonDocument &doc, AsyncClient *client)
{
    JsonDocument response;

    String output;
    serializeJson(response, output);
    output += "\n";
    client->write(output.c_str(), output.length());
};
void DesktopCommManager::_handleGetSettings(JsonDocument &doc, AsyncClient *client)
{
    JsonDocument response;

    String output;
    serializeJson(response, output);
    output += "\n";
    client->write(output.c_str(), output.length());
};

void DesktopCommManager::sendCanLog(const CanFrame &frame, bool isRx)
{
    JsonDocument doc;
    doc["type"] = "can_log";
    doc["dir"] = isRx ? "RX" : "TX";
    doc["id"] = "0x" + String(frame.identifier, HEX);
    JsonArray dataArr = doc["data"].to<JsonArray>();
    for (int i = 0; i < frame.data_length_code; i++)
    {
        dataArr.add(frame.data[i]);
    };
    String output;
    serializeJson(doc, output);
    Serial.println(output);
    output += "\n";
    NetworkManager::getInstance().getTcpServer().sendToAll(output);
};

void DesktopCommManager::sendPMPLog(const String msg, bool isRx)
{
    JsonDocument doc;
    doc["type"] = "pmp_log";
    doc["dir"] = isRx ? "RX" : "TX";
    doc["data"] = msg;
    String output;
    serializeJson(doc, output);
    output += "\n";
    NetworkManager::getInstance().getTcpServer().sendToAll(output);
};

void DesktopCommManager::sendBroadcastStatus()
{
    if (!NetworkManager::getInstance().isConnected())
        return;
    JsonDocument response;
    response["type"] = "status";
    JsonArray servosArr = response["servos"].to<JsonArray>();
    for (auto &[id, servo] : _servos.getServosMap())
    {
        JsonObject s = servosArr.add<JsonObject>();
        s["id"] = id;
        s["pos"] = servo.getParameters(ServoDevice::RealParameter::POSITION);
        s["on"] = servo.isStatusSet(ServoDevice::ServoStatusFlags::ONLINE);
        s["busy"] = servo.isStatusSet(ServoDevice::ServoStatusFlags::BUSY);
        s["curr"] = servo.getParameters(ServoDevice::RealParameter::CURRENT);
        s["temp"] = servo.getParameters(ServoDevice::RealParameter::TEMPERATURE);
        s["err"] = servo.getParameters(ServoDevice::RealParameter::ERROR);
    };
    String output;
    serializeJson(response, output);
    output += "\n";
    NetworkManager::getInstance().getTcpServer().sendToAll(output);
};

// funkcja pomocnicza
IPAddress DesktopCommManager::toIP(String ip_str)
{
    IPAddress ip;
    return ip.fromString(ip_str.c_str());
}