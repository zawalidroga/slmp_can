#include "ServoControl.h"

ServoControl::ServoControl(CanManager &can)
    : can(can), slmp(slmp)
{ // ustawienie onFrame jako funkcji wywołującej parsCanFrame (callback)
    can.onReadFrame = [this](const CanFrame &frame)
    { parseCanRxFrame(frame); }; // przekazanie ramki
    can.onWriteFrame = [this](CanFrame &frame, const int id)
    { parseCanTxFrame(frame, id); }; // przekazanie ramki
    can.deviceNo = [this]()
    { return servos.size(); }; // przekazanie liczby serw
};

void ServoControl::parseCanTxFrame(CanFrame &frame, int id)
{

    ServoDevice *servo = getServo(id);
    const int8_t id = servo->getID();
    const int8_t mode = servo->getServoMode();

    frame.identifier = ((uint16_t)mode << 8) | (id & 0xFF);
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
        frame.data[4] = (servo->speed16 >> 8) & 0xFF;
        frame.data[5] = (servo->speed16 >> 0) & 0xFF;
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
    int value = 0;
    for (int i; i < 7; i++)
    {
        ServoDevice::ParameterServo param = ServoDevice::toParameterServo(i);
        if (i < 6 && !(i % 2))
        {
            value = (frame.data[i + 1] << 8) | frame.data[i];
            servo->setParameters(param, value);
        }
        else if (i > 5)
        {
            value = frame.data[i];
            servo->setParameters(param, value);
        };
    }
};

void ServoControl::setServo(int8_t id)
{
    servos.emplace(id, ServoDevice(id));
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
            idList += ', ';
        };
        idList += String(pair.first);
        firstID = false;
    };
    idList += ".\r\n";
    return idList;
}