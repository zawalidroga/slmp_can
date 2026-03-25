#include "PMPmanager.h"
#include <Arduino.h>
#include "NetworkManager.h"

PMPmanager::PMPmanager(ServoControl &sm) : _servoManager(sm) {};

void PMPmanager::frameHandler(uint8_t *data, size_t packetSize, IPAddress remoteIp, uint16_t remotePort)
{
    _lastPacketTime = millis();
    memcpy(_reqBuffer, data, packetSize);

    if (onPMPframe)
    {

        String msg = " IP: " + remoteIp.toString() + " Dane: ";

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
        NetworkManager::getInstance().sendSystemLog("[PMP] WRONG HEADER!");

        return;
    };

    uint8_t servosNo = _reqBuffer[3];

    if (_reqBuffer[3] == 0)
    {
        NetworkManager::getInstance().sendSystemLog("[PMP] WRONG SERVO NO!");
        endCode = PMP_ERR_INVALID_DATA;
        uint16_t resHeaderLen = _buildResponseHeader(endCode);
        uint16_t totalResLen = resHeaderLen + resPayloadLen;
        return;
    };

    uint8_t *resPayloadPtr = &_resBuffer[6]; // Wskaźnik na początek danych odpowiedzi
    ServoCommandBlock *currentBlock = (ServoCommandBlock *)(_reqBuffer + 4);

    for (size_t i = 0; i < servosNo; i++)
    {
        ServoDevice *servo = _servoManager.getServo(currentBlock->id);
        servo->updateLastCommandTime();
        if (currentBlock->cmd != PMP_CMD_DEVICE_READ)
        {
            if (servo == nullptr)
            {

                endCode = PMP_ERR_INVALID_ID;
                uint16_t resHeaderLen = _buildResponseHeader(endCode);
                uint16_t totalResLen = resHeaderLen + resPayloadLen;
                NetworkManager::getInstance().sendSystemLog("[PMP] NIE POPRAWNE ID SERWA: " + String(currentBlock->id));
                return;
            };
        };
        // NetworkManager::getInstance().sendSystemLog("[PMP] ID: " + String(currentBlock->id) + " komenda: " + String(currentBlock->cmd) + " subkomenda: " + String(currentBlock->subcmd));
        switch (currentBlock->cmd)
        {
        case PMP_CMD_DEVICE_READ:
            switch (currentBlock->subcmd)
            {
            case PMP_SUBCMD_READ_STATUS:
            {
                Serial.println("[PMP] Odczyt parametrów z serwa.");

                break;
            }
            default:
                endCode = PMP_ERR_INVALID_CMD;
                break;
            };
            break;
        case PMP_CMD_DEVICE_WRITE:
        {

            switch (currentBlock->subcmd)
            {
            case PMP_SUBCMD_WRITE_GO:
            {
                int mode = servo->getPositioningMode();
                switch (mode)
                {
                case 0: // tryb jezdy na pozycje
                {
                    NetworkManager::getInstance().sendSystemLog("[PMP] JAZDA!! pozycja: " + String(currentBlock->position) + " predkosc: " + String(currentBlock->speed) + " przyspieszenie: " + String(currentBlock->acceleration));
                    // uint32_t val = (*reqPayloadPtr << 0) | (*(reqPayloadPtr + 1) << 8) | (*(reqPayloadPtr + 2) << 16) | *(reqPayloadPtr + 3) << 24;
                    servo->position = currentBlock->position;
                    servo->speed = currentBlock->speed;
                    servo->acceleration = currentBlock->acceleration;
                }
                break;
                case 1: // tryb jog
                    if (currentBlock->position == 1)
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

                default:
                    // Serial.println("[ERR 05][PMP] Błędny tryb pozycjonowania!");
                    endCode = PSMP_ERR_INVALID_POSITIONING_MODE;
                    servo->setStatus(ServoDevice::ServoStatusFlags::ERROR);
                    break;
                }

                break;
            };
            case PMP_SUBCMD_WRITE_HOMING:
                if (servo->homingStep == HomingState::IDLE)
                {
                    servo->setStatus(ServoDevice::ServoStatusFlags::HPR_BUSY);
                    servo->clearStatus(ServoDevice::ServoStatusFlags::HPR_COMPLETED);
                    servo->homingStep = HomingState::START_HOMING;
                }
                break;
            case PMP_SUBCMD_WRITE_MODE:
            {
                uint8_t mode = currentBlock->position;
                servo->setPositioningMode(mode);
                break;
            }
            case PMP_SUBCMD_WRITE_PARAM:
            {
                // Serial.println("[PMP] Otrzymane dane do zapisu:");
                servo->position = currentBlock->position;
                servo->speed = currentBlock->speed;
                servo->acceleration = currentBlock->acceleration;
                servo->position = currentBlock->position;
            }
            default:
                NetworkManager::getInstance().sendSystemLog("[PMP] WRONG SUBCMD NO!");
                endCode = PMP_ERR_INVALID_CMD;
                break;
            }

            break;
        }
        case PMP_CMD_DEVICE_ONOFF:

            if (currentBlock->subcmd == 1)
            {
                if (!servo->isStatusSet(ServoDevice::ServoStatusFlags::ENABLED))
                {
                    servo->setStatus(ServoDevice::ServoStatusFlags::ENABLED);
                    servo->setServoMode(6);
                    // uint16_t actualPosition = servo->getParameters(ServoDevice::RealParameter::POSITION) * 10000;
                    servo->position = servo->getParameters(ServoDevice::RealParameter::POSITION) * 1000;
                }
            }
            else
            {
                servo->clearStatus(ServoDevice::ServoStatusFlags::ENABLED);
                servo->setServoMode(99);
            }

            break;
        default:
            NetworkManager::getInstance().sendSystemLog("[PMP] WRONG CMD NO!");
            endCode = PMP_ERR_INVALID_CMD;
            break;
        };
        currentBlock++;
    }

    resPayloadLen = _buildReply(resPayloadPtr);

    uint16_t resHeaderLen = _buildResponseHeader(endCode);
    uint16_t totalResLen = resHeaderLen + resPayloadLen;

    if (onPMPframe)
    {
        String msg = " IP: " + remoteIp.toString() + " Dane: ";
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

    // 2. Kopiuj pola docelowe z żądania (bajty 4-5)
    memcpy(&_resBuffer[2], &_reqBuffer[4], 2);

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

// ################ TIMEOUT ################
bool PMPmanager::isTimeout()
{
    // NetworkManager::getInstance().sendSystemLog("[PMP] last packet time: " + String(_lastPacketTime));
    if (_lastPacketTime == 0)
        return false;
    return (millis() - _lastPacketTime > TIMEOUT_PMP_CONNECTION);
}