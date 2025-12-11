#include <Arduino.h>
#include "./network/NetworkManager.h"
#include "./network/AsyncUDPManager.h"
#include "./network/PMPmanager.h"
#include "./network/AsyncTCPServer.h"
#include "./network/CommandParser.h"
#include "./can/ServoControl.h"
#include "./gui/TUIManager.h"

NetworkManager &networkManger = NetworkManager::getInstance();
CanManager canManager;
ServoControl servos(canManager);
AsyncUdpManager &udpManager = networkManger.getUdpManager();
AsyncTcpServer &tcpManager = networkManger.getTcpServer();
CommandManager commManager(servos);
PMPmanager slmpManager(servos);

void setup()
{
    Serial.begin(115200);
    canManager.begin();
    networkManger.begin();
    servos.begin();
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
};

void loop()
{
    delay(10);
};