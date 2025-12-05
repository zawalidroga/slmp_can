#include <Arduino.h>
#include "./network/NetworkManager.h"
#include "./network/AsyncUDPManager.h"
#include "./network/SLMPmanager.h"
#include "./network/AsyncTCPServer.h"
#include "./network/CommandParser.h"
#include "./can/ServoControl.h"
#include "./gui/TUIManager.h"

NetworkManager networkManger;
ServoControl servos;
AsyncUdpManager udpManager;
AsyncTcpServer tcpManager;
CommandManager commManager(servos);
SLMPmanager slmpManager(servos);

void setup()
{
    networkManger.begin();
    slmpManager.sendReply = [](uint8_t *resBuffer, size_t len, IPAddress remoteIp, uint16_t remotePort)
    {
        udpManager.sendFrame(resBuffer, len, remoteIp, remotePort);
    };

    tcpManager.onClientData = [](uint8_t *data, size_t size, AsyncClient *client)
    {
        commManager.dataParser(data, size, client);
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
    // xTaskCreatePinnedToCore();
}

void loop()
{
}
