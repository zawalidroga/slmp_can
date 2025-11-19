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
};

#endif