#include "ServoControl.h"
#include <vector>

ServoControl::ServoControl(CanManager &can)
    : _can(can)
{ // ustawienie onFrame jako funkcji wywołującej parsCanFrame (callback)
    can.onReadFrame = [this](const CanFrame &frame)
    { parseCanRxFrame(frame); }; // przekazanie ramki
    can.onWriteFrame = [this](CanFrame &frame, const int id)
    { parseCanTxFrame(frame, id); }; // przekazanie ramki
    can.deviceNo = [this]()
    { return servos.size(); }; // przekazanie liczby serw
    can.getServosIDs = [this]()
    {
        std::vector<uint8_t> ids;
        for (auto const &[id, servo] : servos)
        {
            ids.push_back(id);
        };
        return ids;
    };
};

void ServoControl::begin()
{
    _loadServoList();
};

void ServoControl::parseCanTxFrame(CanFrame &frame, int id)
{

    ServoDevice *servo = getServo(id);
    // const int8_t id = servo->getID();
    if (servo == nullptr)
    {
        Serial.println("[ServoControl] Brak serwa o podanym ID");
        return;
    };
    const int8_t mode = servo->getServoMode();
    int16_t speed16 = (int16_t)servo->speed;

    frame.identifier = ((uint16_t)mode << 8) | (servo->getID() & 0xFF);
    frame.extd = 1;
    frame.data_length_code = 8;
    switch (mode)
    {
    case CurrentLoop:
        frame.data[0] = (servo->current >> 24) & 0xFF;
        frame.data[1] = (servo->current >> 16) & 0xFF;
        frame.data[2] = (servo->current >> 8) & 0xFF;
        frame.data[3] = (servo->current >> 0) & 0xFF;
        frame.data[4] = 0xAA;
        frame.data[5] = 0xAA;
        frame.data[6] = 0xAA;
        frame.data[7] = 0xAA;
        break;
    case BrakeCurrent:
        frame.data[0] = (servo->brakeCurrent >> 24) & 0xFF;
        frame.data[1] = (servo->brakeCurrent >> 16) & 0xFF;
        frame.data[2] = (servo->brakeCurrent >> 8) & 0xFF;
        frame.data[3] = (servo->brakeCurrent >> 0) & 0xFF;
        frame.data[4] = 0xAA;
        frame.data[5] = 0xAA;
        frame.data[6] = 0xAA;
        frame.data[7] = 0xAA;
        break;
    case VelocityLoop:
        frame.data[0] = (servo->speed >> 24) & 0xFF;
        frame.data[1] = (servo->speed >> 16) & 0xFF;
        frame.data[2] = (servo->speed >> 8) & 0xFF;
        frame.data[3] = (servo->speed >> 0) & 0xFF;
        frame.data[4] = 0xAA;
        frame.data[5] = 0xAA;
        frame.data[6] = 0xAA;
        frame.data[7] = 0xAA;
        break;
    case DutyCycle:
        frame.data[0] = 0xff;
        frame.data[1] = 0xff;
        frame.data[2] = 0xff;
        frame.data[3] = 0xff;
        frame.data[4] = 0xff;
        frame.data[5] = 0xff;
        frame.data[6] = 0xff;
        frame.data[7] = 0xff;
        break;
    case PositionLoop:
        frame.data[0] = (servo->position >> 24) & 0xFF;
        frame.data[1] = (servo->position >> 16) & 0xFF;
        frame.data[2] = (servo->position >> 8) & 0xFF;
        frame.data[3] = (servo->position >> 0) & 0xFF;
        frame.data[4] = 0xAA;
        frame.data[5] = 0xAA;
        frame.data[6] = 0xAA;
        frame.data[7] = 0xAA;
        break;
    case PositionVelocityLoop:

        frame.data[0] = (servo->position >> 24) & 0xFF;
        frame.data[1] = (servo->position >> 16) & 0xFF;
        frame.data[2] = (servo->position >> 8) & 0xFF;
        frame.data[3] = (servo->position >> 0) & 0xFF;
        frame.data[4] = (speed16 >> 8) & 0xFF;
        frame.data[5] = (speed16 >> 0) & 0xFF;
        frame.data[6] = (servo->acceleration >> 8) & 0xFF;
        frame.data[7] = (servo->acceleration >> 0) & 0xFF;
        break;
    case MITVelocityLoop:
        frame.data[0] = 0xAA;
        frame.data[1] = 0xAA;
        frame.data[2] = 0xAA;
        frame.data[3] = 0xAA;
        frame.data[4] = 0xAA;
        frame.data[5] = 0xAA;
        frame.data[6] = 0xAA;
        frame.data[7] = 0xAA;
        break;
    case SetZero:
        frame.data[0] = 0x01;
        frame.data[1] = 0xFF;
        frame.data[2] = 0xFF;
        frame.data[3] = 0xFF;
        frame.data[4] = 0xFF;
        frame.data[5] = 0xFF;
        frame.data[6] = 0xFF;
        frame.data[7] = 0xFF;
        break;
    default:
        break;
    }
};

void ServoControl::parseCanRxFrame(const CanFrame &frame)
{

    const int id = frame.identifier & 0xFF;
    ServoDevice *servo = getServo(id);

    if (servo == nullptr)
    {
        return;
    };
    int value = 0;
    for (int i = 0; i < 7; i++)
    {
        ServoDevice::RealParameter param;
        if (i < 6 && !(i % 2))
        {
            param = ServoDevice::toRealParameter(i / 2 + 2);
            value = (frame.data[i + 1] << 8) | frame.data[i];
            servo->setServoMonitor(param, value);
        }
        else if (i > 5)
        {
            param = ServoDevice::toRealParameter(i);
            value = frame.data[i];
            servo->setServoMonitor(param, value);
        };
    }
};

void ServoControl::setServo(int8_t id, bool save)
{
    if (servos.find(id) == servos.end())
    {
        auto it = servos.emplace(id, ServoDevice(id)).first;
        ServoDevice &newServo = it->second;
        newServo.onStateChanged = [this, id]()
        {
            this->saveServoState(id);
        };
        Serial.printf("[ServoControl] Dodane nowe serwo o ID: %d \n", id);
        if (save)
        {
            saveServoState(id);
            _saveServoList();
        }
    }
};

ServoDevice *ServoControl::getServo(int8_t id)
{
    auto it = servos.find(id);
    if (it != servos.end())
        return &it->second; // find zwraca dwie wartości jako indeks oraz wartość wyszukiwaną
    return nullptr;
};

void ServoControl::parseSLMPFrameRx(int frame) {

};

void ServoControl::parseSLMPFrameTx(int frame) {

};

// ################# ZAPIS SERW DO PAMIĘCI NIEULOTNEJ ####################

void ServoControl::_saveServoList()
{
    _prefs.begin("servos", false);
    String idList = "";
    bool first = true;
    for (auto const &[id, val] : servos)
    {
        if (!first)
            idList += ",";
        idList += String(id);
        first = false;
    };
    Serial.printf("[SAVE] Zapisuję listę ID do klucza 'id_list': '%s'\n", idList.c_str());
    _prefs.putString("id_list", idList);
    Serial.printf("[NVS] Zapisano główną liste serw: %s \n", idList.c_str());

    _prefs.end();
};

void ServoControl::_loadServoList()
{
    if (_prefs.begin("servos", false))
    { // true dla tylko do odczytu
        String idList = "";
        if (_prefs.isKey("id_list"))
        {
            idList = _prefs.getString("id_list", "");
            Serial.printf("[LOAD] Odczytano listę ID: '%s'\n", idList.c_str());
        }
        else
        {
            Serial.println("[LOAD] Klucz 'id_list' nie istnieje.");
        };
        _prefs.end();

        if (idList.length() > 0)
        {
            Serial.println("[LOAD] Przetwarzam odczytaną listę ID...");
            int currentIndex = 0;
            int nextIndex = 0;
            while ((nextIndex = idList.indexOf(',', currentIndex)) != -1)
            {
                String idStr = idList.substring(currentIndex, nextIndex);
                if (idStr.length() > 0)
                {
                    Serial.printf("[LOAD] Tworzę obiekt dla serwa o ID: %s\n", idStr.c_str());
                    setServo(idStr.toInt(), false);
                }
                currentIndex = nextIndex + 1;
            }
            String idStr = idList.substring(currentIndex);
            if (idStr.length() > 0)
            {
                Serial.printf("[LOAD] Tworzę obiekt dla serwa o ID: %s\n", idStr.c_str());
                setServo(idStr.toInt(), false);
            };
            Serial.println("[LOAD] Wczytuję szczegółowy stan dla każdego serwa...");
            for (auto const &[id, val] : servos)
            {
                ServoDevice *servo = getServo(id);
                if (servo)
                {
                    String servoNamespace = "srv_" + String(id);
                    _prefs.begin(servoNamespace.c_str(), false);
                    servo->loadState(_prefs);
                    _prefs.end();
                }
            }
        }
        else
        {
            Serial.println("[LOAD] Lista ID jest pusta, nie ma czego wczytywać.");
        }
    };
};

void ServoControl::saveServoState(uint8_t id)
{
    ServoDevice *servo = getServo(id);
    if (servo)
    {
        String servoNamespace = "srv_" + String(id);
        _prefs.begin(servoNamespace.c_str(), false);
        Serial.printf("[SAVE] Zapisuję stan serwa w przestrzeni '%s'\n", servoNamespace.c_str());
        servo->saveState(_prefs);
        _prefs.end();
    };
};

// ################# FUNKCJE POMOCNICZE #####################

String ServoControl::listServoIDs()
{
    String idList = "Dostępne serwa [ID]: ";

    if (servos.empty())
    {
        idList += "brak dostępnych serw :C";
        return idList;
    };
    bool firstID = true;
    for (const auto &pair : servos)
    {
        if (!firstID)
        {
            idList += ',';
        };
        idList += String(pair.first);
        firstID = false;
    };
    idList += ".\r\n";
    return idList;
}
