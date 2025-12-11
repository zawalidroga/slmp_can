#ifndef ASYNC_UDP_MANAGER_H
#define ASYNC_UDP_MANAGER_H

#include <AsyncUDP.h>
#include <functional>
#include "PMPmanager.h"

class AsyncUdpManager
{
private:
    AsyncUDP udp;

public:
    void start(uint16_t port);

    void sendFrame(uint8_t *resBuffer, size_t len, IPAddress remoteIp, uint16_t remotePort);

    std::function<void(AsyncUDPPacket &)> onFrame;
};

#endif