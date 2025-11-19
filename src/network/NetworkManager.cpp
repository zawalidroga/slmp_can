#include "NetworkManager.h"

void NetworkManager::begin()
{
    WiFi.onEvent(NetworkManager::ethEvent);

    ETH.begin(ETH_ADDR, ETH_POWER_PIN, ETH_MDC_PIN, ETH_MDIO_PIN, ETH_PHY_LAN8720, ETH_CLOCK_GPIO0_IN);
};

void NetworkManager::ethEvent(WiFiEvent_t event)
{
    switch (event)
    {

    case SYSTEM_EVENT_ETH_GOT_IP:
        Serial.print("[ETH] IP obtained: ");
        Serial.println(ETH.localIP());
        _ethConnected = true;

        // START TCP/UDP ASYNC
        NetworkManager::getInstance().TCPserver.start(5020);
        NetworkManager::getInstance().udp.start(5005);
        break;

    case SYSTEM_EVENT_ETH_DISCONNECTED:
        Serial.println("[ETH] Disconnected");
        _ethConnected = false;
        break;

    default:
        break;
    }
};
