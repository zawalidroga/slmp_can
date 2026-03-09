#ifndef ASYNC_TCP_SERVER_H
#define ASYNC_TCP_SERVER_H

#include <AsyncTCP.h>
#include <vector>
#include <algorithm>

class AsyncTcpServer
{
private:
    AsyncServer *server;
    void onClient(AsyncClient *client);
    std::vector<AsyncClient *> _clients;

public:
    void start(uint16_t port);
    std::function<void(uint8_t *, size_t, AsyncClient *)> onClientData;
    std::function<void(AsyncClient *client)> onClientConnect;
    void sendToAll(const String &message);
};

#endif