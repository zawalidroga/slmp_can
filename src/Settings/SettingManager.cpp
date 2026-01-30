#include "SettingManager.h"

SettingMenager::SettingMenager() : _ip(192, 168, 3, 205), _gateway(192, 168, 3, 213), _subnet(255, 255, 255, 0), _primaryDNS(8, 8, 8, 8), _secondaryDNS(8, 8, 4, 4), _portTCP(5020), _portUDP(5005) {};

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

    if (!_preferences.isKey("ip_address"))
    {
        Serial.println("[INFO][Settings] Brak konfiguracji, zapisuję domyślną.");
        save();
    }

    String ipStr = _preferences.getString("ip_address", _ip.toString());
    String gatewayStr = _preferences.getString("gateway_address", _gateway.toString());
    String subnetStr = _preferences.getString("subnet_address", _subnet.toString());
    String primaryStr = _preferences.getString("primary_DNS_address", _primaryDNS.toString());
    String secondaryStr = _preferences.getString("secondary_DNS_address", _secondaryDNS.toString());

    _ip.fromString(ipStr);
    _gateway.fromString(gatewayStr);
    _subnet.fromString(subnetStr);
    _primaryDNS.fromString(primaryStr);
    _secondaryDNS.fromString(secondaryStr);
    _portTCP = _preferences.getUShort("port_tcp", _portTCP);
    _portUDP = _preferences.getUShort("port_udp", _portUDP);

    Serial.println("[INFO][Settings] Ustawienia wczytane");
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
        _preferences.putString("gateway_address", _gateway.toString());
        _preferences.putString("subnet_address", _subnet.toString());
        _preferences.putString("primary_DNS_address", _primaryDNS.toString());
        _preferences.putString("secondary_DNS_address", _secondaryDNS.toString());
        _preferences.putUShort("port_tcp", _portTCP);
        _preferences.putUShort("port_udp", _portUDP);

        Serial.println("[INFO][Settings] Ustawienia zapisane do pamięci flash");
    };
}
