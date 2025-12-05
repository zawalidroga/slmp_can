#include "SLMPmanager.h"
#include <Arduino.h>

SLMPmanager::SLMPmanager(ServoControl &sm) : _servoManager(sm) {};

void SLMPmanager::frameHandler(uint8_t *data, size_t packetSize, IPAddress remoteIp, uint16_t remotePort)
{

    memcpy(_reqBuffer, data, packetSize);

    uint16_t endCode = SLMP_ERR_NONE;
    uint16_t resPayloadLen = 0; // Długość danych *po* End Code
    if (packetSize < 21 || _reqBuffer[0] != 0x50 || _reqBuffer[1] != 0x00)
    {
        Serial.println("Odebrano niewłaściwy pakiet.");
        return;
    };
    // 2. Ekstrakcja pól z żądania
    uint16_t cmd = _reqBuffer[11] | (_reqBuffer[12] << 8);
    uint16_t subcmd = _reqBuffer[13] | (_reqBuffer[14] << 8);
    uint8_t devCode = _reqBuffer[15];
    // Adres jest 3-bajtowy (LSB, M, H)
    uint32_t addr = _reqBuffer[16] | (_reqBuffer[17] << 8);
    uint16_t idServo = _reqBuffer[18];                       /*| (_reqBuffer[20] << 8)*/
    ;                                                        // id serwa wewnetrznie
    uint16_t count = _reqBuffer[19] | (_reqBuffer[20] << 8); // ilość adresów do odczytu/zapisania

    uint8_t *resPayloadPtr = &_resBuffer[11]; // Wskaźnik na początek danych odpowiedzi
    ServoDevice *servo = _servoManager.getServo(idServo);

    if (cmd == SLMP_CMD_DEVICE_READ)
    {
        switch (devCode)
        {
        case SLMP_SERVO_INFO:
            if (subcmd == SLMP_SUBCMD_WORD)
            {
                for (int i = 0; i < count; i++)
                {
                    ServoDevice::RealParameter param = ServoDevice::toRealParameter(i);
                    uint16_t val = servo->getAllParameters(param);
                    *resPayloadPtr++ = (val & 0xFF);
                    *resPayloadPtr++ = (val >> 8);
                }
                resPayloadLen = count * 2;
            }
            else
            {
                endCode = SLMP_ERR_INVALID_CMD;
            }
            break;
        default:
            endCode = SLMP_ERR_INVALID_CMD; // Nieobsługiwane urządzenie
            break;
        };
    }
    else if (cmd == SLMP_CMD_DEVICE_WRITE)
    {
        uint8_t *reqPayloadPtr = &_reqBuffer[21];
        switch (devCode)
        {
        case SLMP_SERVO_PARAM:
            if (subcmd == SLMP_SUBCMD_WORD)
            {
                for (int i = 0; i < count; i++)
                {
                    ServoDevice::ParameterServo param = ServoDevice::toParameterServo(i);
                    if (i < 5)
                    {
                        uint32_t val = *reqPayloadPtr | (*(reqPayloadPtr + 1) << 8) | (*(reqPayloadPtr + 2) << 16) | (*(reqPayloadPtr + 3) << 24);
                        servo->setParameters(param, val); // Zapis do serwa
                        reqPayloadPtr += 4;
                    }
                    else
                    {
                        uint16_t val = *reqPayloadPtr | (*(reqPayloadPtr + 1) << 8);
                        servo->setParameters(param, val); // Zapis do serwa
                        reqPayloadPtr += 2;
                    }
                }
                resPayloadLen = count * 2;
            }
            else
            {
                endCode = SLMP_ERR_INVALID_CMD;
            }
            break;
        default:
            endCode = SLMP_ERR_INVALID_CMD; // Nieobsługiwane urządzenie
            break;
        };
    }
    else
    {
        Serial.printf("Odebrano niewłaściwą komendę - odczytaj / zapisz %04x \r \n", cmd);
    }

    uint16_t resHeaderLen = _buildResponseHeader(resPayloadLen + 2, endCode);
    uint16_t totalResLen = resHeaderLen + resPayloadLen;
    if (sendReply)
    {
        sendReply(_resBuffer, totalResLen, remoteIp, remotePort);
    };
};

uint16_t SLMPmanager::_buildResponseHeader(uint16_t resDataLen, uint16_t endCode)
{
    // 1. Subheader (2 bajty)
    _resBuffer[0] = (uint8_t)(SUBHEADER_RES & 0xFF); // 0xD0
    _resBuffer[1] = (uint8_t)(SUBHEADER_RES >> 8);   // 0x00

    // 2. Kopiuj pola docelowe z żądania (bajty 2-6)
    memcpy(&_resBuffer[2], &_reqBuffer[2], 5);

    // 3. Długość danych odpowiedzi (2 bajty)
    _resBuffer[7] = (uint8_t)(resDataLen & 0xFF);
    _resBuffer[8] = (uint8_t)(resDataLen >> 8);

    // 4. Kod zakończenia (2 bajty)
    _resBuffer[9] = (uint8_t)(endCode & 0xFF);
    _resBuffer[10] = (uint8_t)(endCode >> 8);

    return 11; // Długość nagłówka odpowiedzi
};