#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

#define ETH_ADDR 1
#define ETH_POWER_PIN 16 // GPIO16 dla sterowania zasilaniem/resetem PHY
#define ETH_MDC_PIN 23   // GPIO23 dla zegara zarządzania PHY
#define ETH_MDIO_PIN 18  // GPIO18 dla danych zarządzania PHY
#define ETH_TYPE ETH_PHY_LAN8720

#include "AsyncUDPManager.h"
#include "AsyncTCPServer.h"
#include <Arduino.h>
#include <ETH.h>

class NetworkManager
{
private:
    NetworkManager() = default;
    AsyncUdpManager udp;
    AsyncTcpServer TCPserver;

    static void ethEvent(WiFiEvent_t event);
    static bool _ethConnected;

public:
    static NetworkManager &getInstance();

    void begin();
    void loop();
    bool isConnected() const;

    AsyncUdpManager &getUdpManager();
    AsyncTcpServer &getTcpServer();
};

#endif