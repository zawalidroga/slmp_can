#include "PMPmanager.h"
#include <Arduino.h>

PMPmanager::PMPmanager(ServoControl &sm) : _servoManager(sm) {};

void PMPmanager::frameHandler(uint8_t *data, size_t packetSize, IPAddress remoteIp, uint16_t remotePort)
{

    memcpy(_reqBuffer, data, packetSize);

    Serial.print("IP: ");
    Serial.println(remoteIp);

    Serial.print("PORT: ");
    Serial.println(remotePort);
    Serial.print("Packet Size: ");
    Serial.println(packetSize);
    Serial.println("data: ");
    Serial.println(_reqBuffer[0], HEX);
    Serial.println(_reqBuffer[1], HEX);
    Serial.println(_reqBuffer[2], HEX);
    Serial.println(_reqBuffer[3], HEX);
    Serial.println(_reqBuffer[4], HEX);
    Serial.println(_reqBuffer[5], HEX);
    Serial.println(_reqBuffer[6], HEX);
    Serial.println(_reqBuffer[7], HEX);
    Serial.println(_reqBuffer[8], HEX);
    Serial.println(_reqBuffer[9], HEX);
    Serial.println(_reqBuffer[10], HEX);
    Serial.println(_reqBuffer[11], HEX);

    // return;

    uint8_t endCode = PMP_ERR_NONE;
    uint16_t resPayloadLen = 0; // Długość danych *po* End Code

    if (_reqBuffer[0] != 0x21 || _reqBuffer[1] != 0x37)
    {
        Serial.println("[PMP] Odebrano niewałaściwy pakiet - błędny header");
        return;
    };

    uint8_t cmd = _reqBuffer[2];
    uint8_t subcmd = _reqBuffer[3];
    uint8_t idServo = _reqBuffer[4];

    uint8_t *resPayloadPtr = &_resBuffer[6]; // Wskaźnik na początek danych odpowiedzi
    ServoDevice *servo = _servoManager.getServo(idServo);
    if (servo == nullptr)
    {
        // DODAC ZBUDOWANIE I WYSŁANIE ODPOWIEDZI Z BŁĘDEM;
        Serial.print("[PPM] Niepoprawne ID serwa: ");
        Serial.println(idServo);
        endCode = PMP_ERR_INVALID_ID;
        return;
    };
    switch (cmd)
    {
    case PMP_CMD_DEVICE_READ:
        switch (subcmd)
        {
        case PMP_SUBCMD_READ_STATUS:
        {
            Serial.println("[PMP] Odczyt parametrów z serwa.");
            for (int i = 0; i < 7; i++)
            {

                ServoDevice::RealParameter param = ServoDevice::toRealParameter(i);
                uint16_t val = servo->getAllParameters(param);

                *resPayloadPtr++ = (val >> 8);
                *resPayloadPtr++ = (val & 0xFF);

                Serial.print("Wartość: ");
                Serial.println(val);
                Serial.print("ramka: ");
                Serial.println(*resPayloadPtr, HEX);
            };
            uint16_t val = servo->getServoStatus();
            *resPayloadPtr++ = (val >> 8);
            *resPayloadPtr++ = (val & 0xFF);
            resPayloadLen = 16;
            break;
        }
        default:
            endCode = PMP_ERR_INVALID_CMD;
            break;
        };
        break;
    case PMP_CMD_DEVICE_WRITE:
    {
        uint8_t *reqPayloadPtr = &_reqBuffer[6];
        switch (subcmd)
        {
        case PMP_SUBCMD_WRITE_GO:
        {
            uint32_t val = (*reqPayloadPtr << 24) | (*(reqPayloadPtr + 1) << 16) | (*(reqPayloadPtr + 2) << 8) | *(reqPayloadPtr + 3);
            servo->position = val;
            break;
        }
        case PMP_SUBCMD_WRITE_MODE:

            break;
        case PMP_SUBCMD_WRITE_PARAM:
        {
            Serial.println("[PMP] Otrzymane dane do zapisu:");
            for (int i = 0; i < 7; i++)
            {
                ServoDevice::ParameterServo param = ServoDevice::toParameterServo(i);
                if (i < 5)
                {
                    uint32_t val = (*reqPayloadPtr << 24) | (*(reqPayloadPtr + 1) << 16) | (*(reqPayloadPtr + 2) << 8) | *(reqPayloadPtr + 3);
                    //servo->setParameters(param, val); // Zapis do serwa
                    Serial.println(val);
                    reqPayloadPtr += 4;
                }
                else
                {
                    uint16_t val = (*(reqPayloadPtr) << 8) | *(reqPayloadPtr + 1);
                    //servo->setParameters(param, val); // Zapis do serwa
                    Serial.println(val);
                    reqPayloadPtr += 2;
                }
            }
            break;
        }
        default:
            endCode = PMP_ERR_INVALID_CMD;
            break;
        }

        break;
    }
    case PMP_CMD_DEVICE_ONOFF:
        break;
    default:
        endCode = PMP_ERR_INVALID_CMD;
        break;
    };

    uint16_t resHeaderLen = _buildResponseHeader(endCode);
    uint16_t totalResLen = resHeaderLen + resPayloadLen;
    if (sendReply)
    {
        sendReply(_resBuffer, totalResLen, remoteIp, remotePort);
    };
};

uint16_t PMPmanager::_buildResponseHeader(uint16_t endCode)
{
    // 1. Subheader (2 bajty)
    _resBuffer[0] = (uint8_t)(HEADER_PMP >> 8);   // 0x21
    _resBuffer[1] = (uint8_t)(HEADER_PMP & 0xFF); // 0x37

    // 2. Kopiuj pola docelowe z żądania (bajty 2-4)
    memcpy(&_resBuffer[2], &_reqBuffer[2], 4);

    // 4. Kod zakończenia (2 bajty)
    _resBuffer[5] = endCode;

    return 6; // Długość nagłówka odpowiedzi
};