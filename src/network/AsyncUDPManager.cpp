#include <Arduino.h>
#include "AsyncUDPManager.h"

// w przyszłości do zmiany - asyncudp powinien być całkowicie niezalezny

void AsyncUdpManager::start(uint16_t port)
{
    if (udp.listen(port))
    {
        Serial.printf("[AsyncUDP] Listening on port %d\n", port);

        udp.onPacket([this](AsyncUDPPacket packet)
                     {
                        //int len = packet.length();

                        if (onFrame){
                            onFrame(packet);
                        }; });
    };
};

void AsyncUdpManager::sendFrame(uint8_t *resBuffer, size_t len, IPAddress remoteIp, uint16_t remotePort)
{
    if (resBuffer)
    {
        size_t bytesSent = udp.writeTo(resBuffer, len, remoteIp, remotePort);
        if (bytesSent != len)
        {
            Serial.print("ERROR: UDP send: ");
            Serial.print(bytesSent);
            Serial.print(" bytes. Expected bytes: ");
            Serial.println(len);
        };
    };
};