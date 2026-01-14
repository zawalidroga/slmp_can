#include <Arduino.h>
#include <ArduinoOTA.h>
#include "./network/NetworkManager.h"
#include "./network/AsyncUDPManager.h"
#include "./network/PMPmanager.h"
#include "./network/AsyncTCPServer.h"
#include "./network/CommandParser.h"
#include "./can/ServoControl.h"
#include "./gui/TUIManager.h"
#include "./Settings/SettingManager.h"

NetworkManager &networkManger = NetworkManager::getInstance();
CanManager canManager;
ServoControl servos(canManager);
AsyncUdpManager &udpManager = networkManger.getUdpManager();
AsyncTcpServer &tcpManager = networkManger.getTcpServer();
CommandManager commManager(servos);
PMPmanager slmpManager(servos);

void servoMonitorTask() {};

void setup()
{
    Serial.begin(115200);
    canManager.begin();
    networkManger.begin();

    //-------------------poczekajka na połączenie ----------------
    while (!networkManger.isConnected())
    {
        delay(500);
        Serial.print(".");
    };
    Serial.println("\nEthernet connected!");
    Serial.print("IP address: ");
    Serial.println(ETH.localIP());

    servos.begin();
    SettingMenager::getInstance().begin();
    slmpManager.onLog = [](String msg)
    {
        commManager.getTuiManager().printNetworkMonitor(msg);
    };
    slmpManager.sendReply = [](uint8_t *resBuffer, size_t len, IPAddress remoteIp, uint16_t remotePort)
    {
        NetworkManager::getInstance().getUdpManager().sendFrame(resBuffer, len, remoteIp, remotePort);
    };

    tcpManager.onClientData = [](uint8_t *data, size_t size, AsyncClient *client)
    {
        commManager.dataParser(data, size, client);
        // Serial.println("[onClientData] cośtam się wysyła");
    };

    tcpManager.onClientConnect = [](AsyncClient *client)
    {
        TUIManager &tui = commManager.getTuiManager();

        tui.onNewClientConnect(client);
    };

    udpManager.onFrame = [](AsyncUDPPacket &packet)
    {
        IPAddress remoteIp = packet.remoteIP();
        uint16_t remotePort = packet.remotePort();
        int packetSize = packet.length();
        uint8_t *data = packet.data();

        slmpManager.frameHandler(data, packetSize, remoteIp, remotePort);
    };
    // ------- konfiguracja OTA ---------
    ArduinoOTA.setHostname("polpakex");
    ArduinoOTA.setPassword("1992");

    ArduinoOTA.onStart([]()
                       { Serial.println("OTA Start"); });
    ArduinoOTA.onEnd([]()
                     { Serial.println("\nOTA End"); });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total)
                          { Serial.printf("Progress: %u%%\r", (progress / (total / 100))); });
    ArduinoOTA.onError([](ota_error_t error)
                       {
        Serial.printf("Error[%u]: ", error);
        if(error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
        if(error == OTA_CONNECT_ERROR) Serial.println("Begin Failed");
        if(error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
        if(error == OTA_END_ERROR) Serial.println("End Failed"); });
    ArduinoOTA.begin();
};

void loop()
{
    ArduinoOTA.handle();

    if (commManager.getTuiManager().isMonitoring)
    {
        if (millis() - commManager.getTuiManager().lastStatusUpdate > STATUS_UPDATE_INTERVAL)
        {
            commManager.getTuiManager().lastStatusUpdate = millis();
            commManager.getTuiManager().printServoMonitor();
        };
    };
};