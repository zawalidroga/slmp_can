#ifndef PMP_MANAGER_H
#define PMP_MANAGER_H

// Kody komend PMP (Binary mode)
#define PMP_CMD_DEVICE_ONOFF 0x00 // Załączenie wybranego serwa
#define PMP_CMD_DEVICE_READ 0x01  // Odczyt - u nas odczyt danych z serwa i przekazanie do plc
#define PMP_CMD_DEVICE_WRITE 0x02 // Zapis - parametryzacja ustawień, wywołanie ruchu serwa oraz ustawienia pozycji i prędkości

// Kody subkomend zpisu
#define PMP_SUBCMD_WRITE_MODE 0x00  // wybór trybu działania serwa
#define PMP_SUBCMD_WRITE_PARAM 0x01 // Zapisanie parametrów podstawowych starszy bit określa jaki parametr zapisujemy

#define PMP_SUBCMD_WRITE_GO 0x02 // wywołanie ruchu z nastawą kierunek - JOG, pozycja - jazda na pozycje, natężęnie - jazda do natężęie

// Kody subkomend odczytu
#define PMP_SUBCMD_READ_STATUS 0x00 // odczyt statusów

// Kody błędów PSMP
#define PMP_ERR_NONE 0x00
#define PMP_ERR_INVALID_CMD 0x01 // niepoprawna komenda lub subkomenda
#define PMP_ERR_INVALID_ID 0x02  // niepoprawna id urządzenia
#define PMP_ERR_INVALID_DATA 0x03
#define PMP_ERR_OUT_OF_RANGE 0x04
#define PSMP_ERR_INVALID_POSITIONING_MODE 0x05

// Stałe nałówka
#define HEADER_PMP 0x2137
// #define SUBHEADER_RES 0xD000

#include <functional>
#include "../can/ServoControl.h"

struct ServoCommandBlock
{
    uint8_t id;            // ID serwa
    uint8_t dummy;         //
    uint8_t subcmd;        // np. PMP_SUBCMD_WRITE_GO
    uint8_t cmd;           // np. PMP_CMD_DEVICE_WRITE
    int32_t position;      // 4 bajty - dodatkowo informacje o modzie itp
    int16_t speed;         // 2 bajty
    int16_t acceleration;  // 2 bajty
} __attribute__((packed)); // Razem 11 bajtów na serwo

class PMPmanager
{
private:
    uint8_t _reqBuffer[512];
    uint8_t _resBuffer[512];

    ServoControl &_servoManager;

    void _handlePacket(int packetSize);
    uint16_t _buildResponseHeader(uint16_t endCode);
    void _deviceRead();
    void _deviceWrite();

    uint16_t _buildReply(uint8_t *resPayloadPtr);

    uint16_t _calculate_crc16(const uint8_t *data, size_t length);

public:
    PMPmanager(ServoControl &sm);
    void frameHandler(uint8_t *data, size_t packetSize, IPAddress remoteIp, uint16_t remotePort);

    std::function<void(uint8_t *, size_t, IPAddress, uint16_t)> sendReply;
    std::function<void(const String msg, bool isRx)> onPMPframe;
};

#endif