#ifndef ASYNC_TCP_SERVER_H
#define ASYNC_TCP_SERVER_H

#include <AsyncTCP.h>

class AsyncTcpServer
{
private:
    AsyncServer *server;
    void onClient(AsyncClient *client);

public:
    void start(uint16_t port);
    std::function<void(uint8_t *, size_t, AsyncClient *)> onClientData;
    std::function<void(AsyncClient *client)> onClientConnect;
};

#endif