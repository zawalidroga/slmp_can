#ifndef SLMP_MANAGER_H
#define SLMP_MANAGER_H

// Kody komend SLMP (Binary mode)
#define SLMP_CMD_DEVICE_READ 0x0401  // Odczyt - u nas odczyt danych z serwa i przekazanie do plc
#define SLMP_CMD_DEVICE_WRITE 0x1401 // Zapis - parametryzacja ustawień, wywołanie ruchu serwa oraz ustawienia pozycji i prędkości

// Kody subkomend
#define SLMP_SUBCMD_WORD 0x0000
#define SLMP_SUBCMD_BIT 0x0001

// Kody błędów SLMP
#define SLMP_ERR_NONE 0x0000
#define SLMP_ERR_INVALID_CMD 0xC059 // niepoprawna komenda lub subkomenda
#define SLMP_ERR_INVALID_DEV 0xC05B // niepoprawna kod urządzenia
#define SLMP_ERR_INVALID_DATA 0xC05C
#define SLMP_ERR_OUT_OF_RANGE 0xC051

// Typy urządzeń (device codes)
#define SLMP_SERVO_PARAM 0xA8    // Data register D - u nas parametryzacja serwa (nastawiona prędkość/ pozycja/ współczynniki )
#define SLMP_DEVICE_M 0x90       // Internal relay M
#define SLMP_SERVO_INFO_BIT 0x9C // Input X - informacje o serwie
#define SLMP_SERVO_CONTROL 0x9D  // Output Y - załączanie serwa - wszsystko co
#define SLMP_SERVO_INFO 0xB4     // Link register W - informacje zwrotne serwa (pozycja, prędkośc, natężenie prądu, errory itp.)
#define SLMP_DEVICE_R 0xAF       // File register R

// Stałe nałówka
#define SUBHEADER_CMD 0x5000
#define SUBHEADER_RES 0xD000

#include <functional>
#include "../can/ServoControl.h"

class SLMPmanager
{
private:
    struct SLMP_frame
    {
        uint16_t sub_header;  // 0x5000 (Binary 3E)
        uint8_t network_no;   // 0x00
        uint8_t pc_no;        // 0xFF
        uint16_t io_no;       // 0x03FF
        uint8_t station_no;   // 0x00
        uint16_t data_length; // Długość danych
        uint16_t cpu_timer;   // Timer (zazwyczaj 0x0010)
        uint16_t command;     // Kod komendy
        uint16_t sub_command; // Pod-komenda
    } __attribute__((packed));

    uint8_t _reqBuffer[512];
    uint8_t _resBuffer[512];

    ServoControl &_servoManager;

    void _handlePacket(int packetSize);
    uint16_t _buildResponseHeader(uint16_t resDataLen, uint16_t endCode);
    void _deviceRead();
    void _deviceWrite();

public:
    SLMPmanager(ServoControl &sm);
    void frameHandler(uint8_t *data, size_t packetSize, IPAddress remoteIp, uint16_t remotePort);

    std::function<void(uint8_t *, size_t, IPAddress, uint16_t)> sendReply;
};

#endif