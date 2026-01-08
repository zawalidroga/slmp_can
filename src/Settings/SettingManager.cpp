#include "SettingManager.h"

SettingMenager::SettingMenager() : _ip(192, 168, 1, 100), _portTCP(5005), _portUDP(5020) {};

SettingMenager &SettingMenager::getInstance()
{
    static SettingMenager instance;
    return instance;
};

void SettingMenager::begin()
{
    if (!_preferences.begin("settings", false))
    {
        Serial.println("[WARN][Settigs] Nie udało się otworzyc Preferences do zapisu.");
        return;
    }
    else
    {
        String ipStr = _preferences.getString("ip_address", _ip.toString());
        _ip.fromString(ipStr);
        _portTCP = _preferences.getUShort("port_tcp", _portTCP);
        _portUDP = _preferences.getUShort("port_udp", _portUDP);

        Serial.println("[INFO][Settings] Ustawienia wczytane");
    };
};

void SettingMenager::save()
{
    if (!_preferences.begin("settings", false))
    {
        Serial.println("[WARN][Settigs] Nie udało się otworzyc Preferences do zapisu.");
        return;
    }
    else
    {
        _preferences.putString("ip_address", _ip.toString());
        _preferences.putUShort("port_tcp", _portTCP);
        _preferences.putUShort("port_udp", _portUDP);

        Serial.println("[INFO][Settings] Ustawienia zapisane do pamięci flash");
    };
}
