#include "NetworkManager.h"
#include "../Settings/SettingManager.h"

bool NetworkManager::_ethConnected = false;

NetworkManager &NetworkManager::getInstance()
{
    static NetworkManager instance;
    return instance;
}

void NetworkManager::begin()
{
    WiFi.mode(WIFI_OFF);
    WiFi.onEvent(NetworkManager::ethEvent);

    if (!ETH.begin(ETH_ADDR, ETH_POWER_PIN, ETH_MDC_PIN, ETH_MDIO_PIN, ETH_PHY_LAN8720, ETH_CLOCK_GPIO0_IN))
    {
        Serial.println("FATAL: Inicjalizacja sprzętu ETH nie powiodła się!");
        return;
    };

    IPAddress local_IP(192, 168, 3, 205);
    IPAddress gateway(192, 168, 3, 213);
    IPAddress subnet(255, 255, 255, 0);
    IPAddress primaryDNS(8, 8, 8, 8);
    IPAddress secondaryDNS(8, 8, 4, 4);

    if (!ETH.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS))
    {
        Serial.println("Błąd: nie udało się skonfigurować IP");
        return;
    };

    Serial.println("Konfiguracja IP zakończona, uruchamiam serwery TCP/UDP...");
    TCPserver.start(5020);
    // udp.start(5055);

    // _ethConnected = true;
};

void NetworkManager::ethEvent(WiFiEvent_t event)
{
    Serial.print("ten ivent: ");
    Serial.println(event);
    switch (event)
    {
    case ARDUINO_EVENT_ETH_START:
        Serial.println("[ETH] Started!");
        ETH.setHostname("polpakex");
        break;
    case ARDUINO_EVENT_ETH_CONNECTED:
        Serial.println("[ETH] Connected!");
        _ethConnected = true;
        break;
    case ARDUINO_EVENT_ETH_GOT_IP:
        Serial.print("[ETH] IP obtained: ");
        Serial.println(ETH.localIP());
        _ethConnected = true;

        // START TCP/UDP ASYNC
        NetworkManager::getInstance().TCPserver.start(5020);
        NetworkManager::getInstance().udp.start(5005);
        break;

    case ARDUINO_EVENT_ETH_DISCONNECTED:
        Serial.println("[ETH] Disconnected");
        _ethConnected = false;
        break;
    case ARDUINO_EVENT_ETH_STOP:
        Serial.println("[ETH] Stopped.");
        break;

    default:
        Serial.println("[ETH] coś nie tak ");
        break;
    }
};

bool NetworkManager::isConnected() const
{
    return _ethConnected;
}

AsyncUdpManager &NetworkManager::getUdpManager()
{
    return udp;
};
AsyncTcpServer &NetworkManager::getTcpServer()
{
    return TCPserver;
};

void NetworkManager::loop() {};
