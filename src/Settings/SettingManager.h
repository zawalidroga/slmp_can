#ifndef SETTING_MENAGER_H
#define SETTING_MENAGER_H

#include <Arduino.h>
#include <Preferences.h>

class SettingMenager
{
private:
    IPAddress _ip = IPAddress(192, 168, 1, 100);
    uint8_t _portTCP = 5005;
    uint8_t _portUDP = 5020;

    SettingMenager();

    Preferences _preferences;

public:
    static SettingMenager &getInstance();

    void begin();

    void save();

    void setIPAdrdess(IPAddress ipAdress);
    void getIPAddress();
    void setPort(uint8_t port);
    void getPort();
};

#endif