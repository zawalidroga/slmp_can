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

    _canSendQueue = xQueueCreate(30, sizeof(CanFrame));

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
            NetworkManager::getInstance().sendSystemLog("[CAN] Błąd uruchomienia CAN!");

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
    NetworkManager::getInstance().sendSystemLog("[CAN] CAN Uruchomiony. Oczekiwanie na ramki...");
};

void CanManager::readFrame()
{
    // Serial.println("[CAN] odbiera");
    if (ESP32Can.readFrame(rxFrame, 1000))
    {
        onReadFrame(rxFrame);
    };
};

bool CanManager::sendFrameAsync(const CanFrame &frame)
{
    if (_canSendQueue == NULL)
        return false;
    // Wrzuca ramkę natychmiast (0 opóźnienia), aby nie blokować obliczeń
    return (xQueueSend(_canSendQueue, &frame, 0) == pdTRUE);
}

void CanManager::_canSendTask(void *pvParameters)
{
    CanManager *manager = (CanManager *)pvParameters;
    CanFrame frameToSend;

    for (;;)
    {
        // Wątek czeka w uśpieniu na pojawienie się ramki w kolejce
        if (xQueueReceive(manager->_canSendQueue, &frameToSend, portMAX_DELAY) == pdTRUE)
        {
            if (ESP32Can.writeFrame(frameToSend, pdMS_TO_TICKS(1)) != ESP_OK)
            {
                // Błąd wysyłki (można dodać logowanie)
            }
        }
    }
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
