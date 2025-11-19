#include "CanManager.h"

void CanManager::begin()
{
    // Serial.begin(115200);
    Serial.println("Start programu - CubeMars CAN Reader...");
    ESP32Can.setPins(CAN_TX_PIN, CAN_RX_PIN);
    ESP32Can.setRxQueueSize(20);
    ESP32Can.setTxQueueSize(20);

    _fConfig.acceptance_code = 0x00002800;
    _fConfig.acceptance_mask = 0xffffff00; // trzeba zmienić
    _fConfig.single_filter = true;

    ESP32Can.setSpeed(ESP32Can.convertSpeed(1000));

    if (!ESP32Can.begin())
    {
        while (true)
        {
            Serial.println("Błąd uruchomienia CAN!");
            delay(5000);
        }
    };

    Serial.println("CAN Uruchomiony. Oczekiwanie na ramki...");
};

void CanManager::readFrame()
{
    if (ESP32Can.readFrame(rxFrame, 1000))
    {
        onReadFrame(rxFrame);
    };
};

void CanManager::writeFrame()
{
    vTaskDelay(pdMS_TO_TICKS(1000));
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(_send_interval);

    for (;;)
    {
        for (int i = 0; i < deviceNo(); i++)
        {
            onWriteFrame(txFrame, i);
            ESP32Can.writeFrame(txFrame, 1000);
        }
    }
};