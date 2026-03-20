#include "CanManager.h"
#include "../network/NetworkManager.h"

void CanManager::begin()
{
    // Serial.begin(115200);
    Serial.println("Start programu - CubeMars CAN Reader...");
    ESP32Can.setPins(CAN_TX_PIN, CAN_RX_PIN);
    ESP32Can.setRxQueueSize(20);
    ESP32Can.setTxQueueSize(20);
    ESP32Can.setSpeed(ESP32Can.convertSpeed(1000));

    _fConfig.acceptance_code = 0x00002800;
    _fConfig.acceptance_mask = 0xffffffff; // trzeba zmienić
    _fConfig.single_filter = true;

    if (!ESP32Can.begin())
    {
        while (true)
        {
            Serial.println("Błąd uruchomienia CAN!");

            delay(5000);
        }
    };

    twai_start();

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
    //Serial.println("[CAN] odbiera");
    if (ESP32Can.readFrame(rxFrame, 1000))
    {

       // NetworkManager::getInstance().sendSystemLog("[CAN] odczytuje");
        //Serial.println("[CAN] odczytuje ramke");
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
            std::vector<uint8_t> servoIDs = manager->getAllServosIDs ? manager->getAllServosIDs() : std::vector<uint8_t>();
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
    twai_status_info_t status_info;
    for (;;)
    {
        // if (twai_clear_receive_queue() == ESP_OK)
        // {
        //     if (status_info.state == TWAI_STATE_BUS_OFF)
        //     {
        //         // NetworkManager::getInstance().sendSystemLog("[CAN] ESP OK");
        //         Serial.println("[CAN] ESP down");
        //     }
        //     else if (status_info.state == TWAI_STATE_STOPPED)
        //     {
        //         // NetworkManager::getInstance().sendSystemLog("[CAN ERR] ESP STOPPED!!!");
        //         Serial.println("[CAN] ESP stopped");
        //     }
        //     else if (status_info.state == TWAI_STATE_RUNNING)
        //     {
        //         // NetworkManager::getInstance().sendSystemLog("[CAN ERR] ESP RUNNING!!!");
        //         Serial.println("[CAN] ESP running");
        //     }
        // }

        manager->readFrame();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
