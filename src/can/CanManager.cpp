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
    // if (_canSendQueue == NULL)
    // {
    //     Serial.println("FATAL: Nie udało się utworzyć kolejki CAN!");
    // };

    xTaskCreatePinnedToCore(
        CanManager::_canSendTask,
        "CANSendTask",
        4096,
        this,
        2,
        &_sendTaskHandle,
        0);

    xTaskCreatePinnedToCore(
        CanManager::_canReadTask,
        "CANReadTask",
        4096,
        this,
        1,
        &_readTaskHandle,
        0);

    Serial.println("CAN Uruchomiony. Oczekiwanie na ramki...");
};

void CanManager::readFrame()
{
    if (ESP32Can.readFrame(rxFrame, 1000))
    {
        onReadFrame(rxFrame);
    };
};

void CanManager::_canSendTask(void *pvParameters)
{
    CanManager *manager = (CanManager *)pvParameters;
    CanFrame frameToSend;

    for (;;)
    {

        if (manager->onWriteFrame && manager->deviceNo)
        {
            std::vector<uint8_t> servoIDs = manager->getOnlineServosIDs();
            if (!servoIDs.empty())
            {
                for (uint8_t id : servoIDs)
                {
                    manager->onWriteFrame(manager->txFrame, id);
                    ESP32Can.writeFrame(manager->txFrame);
                    vTaskDelay(pdMS_TO_TICKS(5)); // robi mini delay żeby nie zapchać magistrali
                }
            }
        };
        vTaskDelay(pdMS_TO_TICKS(5));
    };
};

void CanManager::_canReadTask(void *pvParameters)
{
    CanManager *manager = (CanManager *)pvParameters;
    for (;;)
    {
        manager->readFrame();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
