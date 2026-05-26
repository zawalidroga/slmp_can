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
    // Ustawienie filtra tak, aby akceptował wszystkie ramki CAN.
    // Jest to najlepsze rozwiązanie na etapie deweloperskim.
    _fConfig.acceptance_code = 0;
    _fConfig.acceptance_mask = 0;
    _fConfig.single_filter = true;

    if (!ESP32Can.begin())
    {
        while (true)
        {
            Serial.println("Błąd uruchomienia CAN!");

            delay(5000);
        }
    };

    // twai_start(); // Ta linia jest prawdopodobnie zbędna, ESP32Can.begin() powinno już to robić.

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
    // Serial.println("[CAN] odbiera");
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
            std::vector<uint8_t> servoIDs = manager->getAllServosIDs ? manager->getAllServosIDs() : std::vector<uint8_t>();
            if (!servoIDs.empty())
            {
                for (uint8_t id : servoIDs)
                {
                    if (manager->onWriteFrame(manager->txFrame, id))
                    {

                        if (ESP32Can.writeFrame(manager->txFrame, pdMS_TO_TICKS(1)) != ESP_OK)
                        {
                            // Opcjonalnie: obsługa błędu, gdy kolejka TX jest pełna
                        }
                        // vTaskDelay(pdMS_TO_TICKS(1)); // Ten wewnętrzny delay nie jest konieczny, jeśli główna pętla ma wystarczające opóźnienie
                    }
                }
            }
        };
        vTaskDelay(pdMS_TO_TICKS(10)); // Zwiększamy opóźnienie, aby zmniejszyć częstotliwość odpytywania i odciążyć system
    };
};

void CanManager::_canReadTask(void *pvParameters)
{
    CanManager *manager = (CanManager *)pvParameters;
    twai_status_info_t status_info;
    for (;;)
    {
        twai_get_status_info(&status_info);
        if (status_info.state == TWAI_STATE_BUS_OFF)
        {
            NetworkManager::getInstance().sendSystemLog("[CAN] BUS OFF detected! Recovering...");
            twai_initiate_recovery();
            vTaskDelay(pdMS_TO_TICKS(100));
            twai_start();
        };
        while (ESP32Can.readFrame(manager->rxFrame, 0))
        {
            manager->onReadFrame(manager->rxFrame);
        }
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}
