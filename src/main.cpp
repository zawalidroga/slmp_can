#include <Arduino.h>
#include "./network/AsyncUDPManager.h"
#include "./network/SLMPmanager.h"
#include "./can/ServoControl.h"

ServoControl servos;
AsyncUdpManager udpManager;
SLMPmanager slmpManager(servos);

void setup()
{
    slmpManager.sendReply = [](uint8_t *resBuffer, size_t len, IPAddress remoteIp, uint16_t remotePort)
    {
        udpManager.sendFrame(resBuffer, len, remoteIp, remotePort);
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
