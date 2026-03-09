#include "PMPmanager.h"
#include <Arduino.h>

PMPmanager::PMPmanager(ServoControl &sm) : _servoManager(sm) {};

void PMPmanager::frameHandler(uint8_t *data, size_t packetSize, IPAddress remoteIp, uint16_t remotePort)
{

    memcpy(_reqBuffer, data, packetSize);

    for (int i = 0; i < 16; i++)
    {
        Serial.println(_reqBuffer[i], HEX);
    }
    if (onPMPframe)
    {

        String msg = "IP: " + String(remoteIp) + "Dane: ";
        for (size_t i = 0; i < packetSize && i < 32; i++)
        {
            char hex[4];
            sprintf(hex, "%02X ", data[i]);
            msg += hex;
        }
        onPMPframe(msg, false);
    };
    // return;

    uint8_t endCode = PMP_ERR_NONE;
    uint16_t resPayloadLen = 0; // Długość danych po End Code

    if (_reqBuffer[1] != 0x21 || _reqBuffer[0] != 0x37)
    {
        // Serial.println("[PMP] Odebrano niewałaściwy pakiet - błędny header");
        return;
    };

    uint8_t cmd = _reqBuffer[3];
    uint8_t subcmd = _reqBuffer[2];
    uint8_t idServo = _reqBuffer[5];
    ServoDevice *servo;

    uint8_t *resPayloadPtr = &_resBuffer[6]; // Wskaźnik na początek danych odpowiedzi
    if (cmd != PMP_CMD_DEVICE_READ)
    {
        servo = _servoManager.getServo(idServo);
        if (servo == nullptr)
        {
            // DODAC ZBUDOWANIE I WYSŁANIE ODPOWIEDZI Z BŁĘDEM;
            // Serial.print("[PPM] Niepoprawne ID serwa: ");
            // Serial.println(idServo);
            endCode = PMP_ERR_INVALID_ID;
            return;
        };
    };
    switch (cmd)
    {
    case PMP_CMD_DEVICE_READ:
        switch (subcmd)
        {
        case PMP_SUBCMD_READ_STATUS:
        {
            Serial.println("[PMP] Odczyt parametrów z serwa.");
            resPayloadLen = _buildReply(resPayloadPtr);
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
            int mode = servo->getPositioningMode();
            switch (mode)
            {
            case 0: // tryb jezdy na pozycje
            {
                uint32_t val = (*reqPayloadPtr << 0) | (*(reqPayloadPtr + 1) << 8) | (*(reqPayloadPtr + 2) << 16) | *(reqPayloadPtr + 3) << 24;
                servo->position = val;
                Serial.print("Serwo ruszyło na pozycje: ");
                Serial.println(val);
            }
            break;
            case 1: // tryb jog
                if (*reqPayloadPtr == 1)
                {
                    // servo -> speed = (*(reqPayloadPtr + 2) << 0) | (*(reqPayloadPtr + 3) << 8) | (*(reqPayloadPtr + 4) << 16) | *(reqPayloadPtr + 5) << 24;
                    servo->setServoMode(2);
                    servo->position = servo->getParameters(ServoDevice::RealParameter::POSITION);
                }
                else
                {
                    servo->position = servo->getParameters(ServoDevice::RealParameter::POSITION);
                    servo->setServoMode(6);
                }
                break;
            case 2: // tryb bazowania
                break;
            default:
                // Serial.println("[ERR 05][PMP] Błędny tryb pozycjonowania!");
                endCode = PSMP_ERR_INVALID_POSITIONING_MODE;
                servo->setStatus(ServoDevice::ServoStatusFlags::ERROR);
                break;
            }

            break;
        };
        case PMP_SUBCMD_WRITE_MODE:
        {
            uint8_t mode = *reqPayloadPtr;
            servo->setPositioningMode(mode);
            break;
        }
        case PMP_SUBCMD_WRITE_PARAM:
        {
            // Serial.println("[PMP] Otrzymane dane do zapisu:");
            for (int i = 0; i < 7; i++)
            {
                ServoDevice::ParameterServo param = ServoDevice::toParameterServo(i);
                if (i < 5)
                {
                    uint32_t val = (*reqPayloadPtr << 0) | (*(reqPayloadPtr + 1) << 8) | (*(reqPayloadPtr + 2) << 16) | *(reqPayloadPtr + 3) << 24;
                    servo->setParameters(param, val); // Zapis do serwa
                    Serial.println(val);
                    reqPayloadPtr += 4;
                }
                else
                {
                    uint16_t val = (*(reqPayloadPtr) << 0) | *(reqPayloadPtr + 1) << 8;
                    servo->setParameters(param, val); // Zapis do serwa
                    Serial.println(val);
                    reqPayloadPtr += 2;
                }
            };

            break;
        }
        default:
            endCode = PMP_ERR_INVALID_CMD;
            break;
        }
        resPayloadLen = _buildReply(resPayloadPtr);
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

    if (onPMPframe)
    {

        String msg = "IP: " + String(remoteIp) + "Dane: ";
        for (size_t i = 0; i < totalResLen && i < 32; i++)
        {
            char hex[4];
            sprintf(hex, "%02X ", _resBuffer[i]);
            msg += hex;
        }
        onPMPframe(msg, true);
    };

    if (sendReply)
    {
        sendReply(_resBuffer, totalResLen, remoteIp, remotePort);
    };
};

uint16_t PMPmanager::_buildResponseHeader(uint16_t endCode)
{

    uint8_t servosNumber = _servoManager.getServoIds().size();
    // 1. Subheader (2 bajty)
    _resBuffer[1] = (uint8_t)(HEADER_PMP >> 8);   // 0x21
    _resBuffer[0] = (uint8_t)(HEADER_PMP & 0xFF); // 0x37

    // 2. Kopiuj pola docelowe z żądania (bajty 2-3)
    memcpy(&_resBuffer[2], &_reqBuffer[2], 2);

    // 4. Kod zakończenia (2 bajty)
    _resBuffer[5] = servosNumber; // liczba serw
    _resBuffer[4] = endCode;

    return 6; // Długość nagłówka odpowiedzi
};

uint16_t PMPmanager::_buildReply(uint8_t *resPayloadPtr)
{
    uint8_t *payloadStart = resPayloadPtr;
    uint16_t respondLength = 0;
    std::vector<uint8_t> ids = _servoManager.getServoIds();
    for (auto const &id : ids)
    {
        ServoDevice *servo = _servoManager.getServo(id);
        if (servo == nullptr)
        {
            continue;
        };
        for (int i = 0; i <= static_cast<int>(ServoDevice::RealParameter::TEMPERATURE); i++)
        {

            ServoDevice::RealParameter param = ServoDevice::toRealParameter(i);
            uint16_t val = servo->getParameters(param);

            // Serial.printf("[%d] ", i);
            // Serial.print("Wartość: ");
            // Serial.println(val);
            // Serial.print("ramka: ");
            // Serial.println((val & 0xFF) | (val >> 8), HEX);
            *resPayloadPtr++ = (val & 0xFF);
            *resPayloadPtr++ = (val >> 8);
        };
        uint16_t val = servo->getServoStatus();

        *resPayloadPtr++ = (val & 0xFF);
        *resPayloadPtr++ = (val >> 8);
    };
    uint16_t payloadLength = resPayloadPtr - payloadStart;
    uint16_t crc = _calculate_crc16(payloadStart, payloadLength);
    *resPayloadPtr++ = (crc & 0xFF);
    *resPayloadPtr++ = (crc >> 8);
    return payloadLength + 2;
};

// ############ SUMA KONTROLNA #############
uint16_t PMPmanager::_calculate_crc16(const uint8_t *data, size_t length)
{
    uint16_t crc = 0xFFFF;
    for (size_t i = 0; i < length; i++)
    {
        crc ^= (uint16_t)data[i] << 8;
        for (uint8_t j = 0; j < 8; j++)
        {
            if (crc & 0x8000)
            {
                crc = (crc << 1) ^ 0x1021;
            }
            else
            {
                crc <<= 1;
            };
        };
    };
    return crc;
};