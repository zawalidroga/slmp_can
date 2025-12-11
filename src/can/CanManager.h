#ifndef CAN_MANAGER_H
#define CAN_MANAGER_H

#include <ESP32-TWAI-CAN.hpp>
#include "freertos/queue.h"
#include <vector>

#define CAN_TX_PIN GPIO_NUM_5
#define CAN_RX_PIN GPIO_NUM_4

#define KEEP_ALIVE_INTERVAL_MS 50 // ms

class CanManager
{
public:
    void begin();
    void readFrame();

    std::function<void(const CanFrame &)> onReadFrame; // callback - do funckji onFrame przypisuje odpowiednią funkcje która w odpowiedni sposób obrobi ramkę rozkładając ją na potrzebne dane
    std::function<void(CanFrame &, const int)> onWriteFrame;
    std::function<int()> deviceNo;
    std::function<std::vector<uint8_t>()> getServosIDs;

private:
    twai_filter_config_t _fConfig;
    // int _send_interval;
    CanFrame rxFrame;
    CanFrame txFrame;

    QueueHandle_t _canSendQueue;

    // ewentualnie do rozwijania w przyszłości aplikacji
    TaskHandle_t _sendTaskHandle;
    TaskHandle_t _readTaskHandle;
    // dotąd

    static void _canSendTask(void *pvParameters);
    static void _canReadTask(void *pvParameters);
};

#endif