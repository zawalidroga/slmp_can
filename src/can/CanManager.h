#ifndef CAN_MANAGER_H
#define CAN_MANAGER_H

#include <ESP32-TWAI-CAN.hpp>

#define CAN_TX_PIN GPIO_NUM_5
#define CAN_RX_PIN GPIO_NUM_4

class CanManager
{
public:
    void begin();
    void readFrame();
    void writeFrame();
    std::function<void(const CanFrame &)> onReadFrame; // callback - do funckji onFrame przypisuje odpowiednią funkcje która w odpowiedni sposób obrobi ramkę rozkładając ją na potrzebne dane
    std::function<void(CanFrame &, const int)> onWriteFrame;
    std::function<int()> deviceNo;

private:
    twai_filter_config_t _fConfig;
    int _send_interval;
    CanFrame rxFrame;
    CanFrame txFrame;
};

#endif