#include "AsyncTCPServer.h"
#include <Arduino.h>

void AsyncTcpServer::start(uint16_t port)
{
    if (!server)
    {
        server = new AsyncServer(port);

        server->onClient([this](void *arg, AsyncClient *client)
                         { this->onClient(client); }, nullptr);

        server->begin();
        Serial.printf("[AsyncTCP] Server started on port %d\n", port);
    }
};

void AsyncTcpServer::onClient(AsyncClient *client)
{
    Serial.println("[AsyncTCP] Client connected");

    client->onData([](void *arg, AsyncClient *c, void *data, size_t len)
                   {
                       uint8_t *d = (uint8_t *)data;
                       Serial.printf("[AsyncTCP] RX (%d bytes): ", len);
                       for (size_t i = 0; i < len; i++)
                           Serial.printf("%02X ", d[i]);
                       Serial.println();

                       // echo back
                       c->write((const char*)data, len); },
                   nullptr);

    client->onDisconnect([](void *arg, AsyncClient *c)
                         { Serial.println("[AsyncTCP] Client disconnected"); }, nullptr);
}