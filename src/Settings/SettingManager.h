#ifndef SETTING_MENAGER_H
#define SETTING_MENAGER_H

#include <Arduino.h>
#include <Preferences.h>

class SettingMenager
{
private:
    IPAddress _ip = IPAddress(192, 168, 3, 205);
    IPAddress _gateway = IPAddress(192, 168, 3, 213);
    IPAddress _subnet = IPAddress(255, 255, 255, 0);
    IPAddress _primaryDNS = IPAddress(8, 8, 8, 8);
    IPAddress _secondaryDNS = IPAddress(8, 8, 4, 4);
    uint16_t _portTCP = 5020;
    uint16_t _portUDP = 5005;

    SettingMenager();

    Preferences _preferences;

public:
    static SettingMenager &getInstance();

    void begin();

    void save();

    void setIPAdrdess(IPAddress ipAdress);
    void setGateway(IPAddress gateway);
    void setSubnet(IPAddress subnet);
    void setPrimaryDNS(IPAddress dns);
    void setSecondaryDNS(IPAddress dns);
    void setTCPPort(uint8_t port);
    void setUDPPort(uint8_t port);

    IPAddress getIPAddress();
    IPAddress getGateway();
    IPAddress getSubnet();
    IPAddress getPrimaryDNS();
    IPAddress getSecondaryDNS();
    uint16_t getUDPPort();
    uint16_t getTCPPort();
};

#endif