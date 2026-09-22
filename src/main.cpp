#include <Arduino.h>
#include <ArduinoOTA.h>
#include "./network/NetworkManager.h"
#include "./network/AsyncUDPManager.h"
#include "./network/PMPmanager.h"
#include "./network/AsyncTCPServer.h"
#include "./network/DesktopCommManager.h"
#include "./can/ServoControl.h"
#include "./Settings/SettingManager.h"
#include "./gui/StatusLeds.h"

#define SERVO_CONNECTION_CKECK_INTERVAL 3000 // 3s
#define STATUS_UPDATE_INTERVAL 1000          // 3s

NetworkManager &networkManager = NetworkManager::getInstance();
CanManager canManager;
ServoControl servos(canManager);
AsyncUdpManager &udpManager = networkManager.getUdpManager();
AsyncTcpServer &tcpManager = networkManager.getTcpServer();
PMPmanager pmpManager(servos);
DesktopCommManager desktopCommManager(servos);
StatusLEDs ledsManager;

unsigned long lastServoConnectionCheck = 0;
unsigned long lastStatusUpdate = 0;

void servoMonitorTask() {};

void setup()
{
    Serial.begin(115200);
    pinMode(36, INPUT);
    pinMode(35, INPUT);
    pinMode(39, INPUT);

    // pinMode(14, OUTPUT); // czerwony
    // pinMode(12, OUTPUT); // pomarańczowy
    // pinMode(15, OUTPUT); // zielony

    SettingMenager::getInstance().begin();
    ledsManager.begin();
    canManager.begin();
    networkManager.begin();

    //-------------------poczekajka na połączenie ----------------
    while (!networkManager.isConnected())
    {
        delay(500);
        Serial.print(".");
    };
    ledsManager.setHeartbeat(true);
    Serial.println("\nEthernet connected!");
    Serial.print("IP address: ");
    Serial.println(ETH.localIP());

    servos.begin();

    pmpManager.sendReply = [](uint8_t *resBuffer, size_t len, IPAddress remoteIp, uint16_t remotePort)
    {
        NetworkManager::getInstance().getUdpManager().sendFrame(resBuffer, len, remoteIp, remotePort);
    };

    tcpManager.onClientData = [](uint8_t *data, size_t size, AsyncClient *client)
    {
        desktopCommManager.dataParser(data, size, client);
        // networkManager.sendSystemLog("[onClientData] cośtam się wysyła");
    };

    tcpManager.onClientConnect = [](AsyncClient *client)
    {
        // TUIManager &tui = commManager.getTuiManager();

        // tui.onNewClientConnect(client);
    };

    udpManager.onFrame = [](AsyncUDPPacket &packet)
    {
        IPAddress remoteIp = packet.remoteIP();
        uint16_t remotePort = packet.remotePort();
        int packetSize = packet.length();
        uint8_t *data = packet.data();

        pmpManager.frameHandler(data, packetSize, remoteIp, remotePort);
        ledsManager.notifyPMPActivity();
    };

    servos.onCanFrame = [](const CanFrame &frame, bool isRx)
    {
        // desktopCommManager.sendCanLog(frame, isRx);
        ledsManager.notifyCanActivity();
    };

    pmpManager.onPMPframe = [](const String msg, bool isRx)
    {
        desktopCommManager.sendPMPLog(msg, isRx);
    };

    // ------- konfiguracja OTA ---------
    ArduinoOTA.setHostname("polpakex");
    ArduinoOTA.setPassword("1992");

    ArduinoOTA.onStart([]()
                       { Serial.println("OTA Start"); });
    ArduinoOTA.onEnd([]()
                     { Serial.println("\nOTA End"); });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total)
                          { Serial.printf("Progress: %u%%\r", (progress / (total / 100))); });
    ArduinoOTA.onError([](ota_error_t error)
                       {
        Serial.printf("Error[%u]: ", error);
        if(error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
        if(error == OTA_CONNECT_ERROR) Serial.println("Begin Failed");
        if(error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
        if(error == OTA_END_ERROR) Serial.println("End Failed"); });
    ArduinoOTA.begin();
};

void loop()
{
    ArduinoOTA.handle();

    if (millis() - lastServoConnectionCheck > SERVO_CONNECTION_CKECK_INTERVAL)
    {

        lastServoConnectionCheck = millis();
        servos.checkServoConnection();
    }

    if (millis() - lastStatusUpdate > STATUS_UPDATE_INTERVAL)
    {
        lastStatusUpdate = millis();
        if (networkManager.isConnected())
        {

            desktopCommManager.sendBroadcastStatus();
        }
    };

    for (auto const &[id, servo] : servos.getServosMap())
    {
        bool sensor = digitalRead(36) == HIGH;
        bool sensor1 = digitalRead(35) == HIGH;
        bool sensor2 = digitalRead(39) == HIGH;
        ServoDevice *s = servos.getServo(id);
        if (s->isCommandTimeout(30000))
        {
            s->clearStatus(ServoDevice::ServoStatusFlags::ENABLED);
            s->setServoMode(99);
            NetworkManager::getInstance().sendSystemLog("[MAIN] Timeout komunikacji Serva! Serwa wyłączone. ID: " + String(id));
        }
        if (s)
        {
            switch (id)
            {
            case 1:
                s->makeItHome(sensor);
                break;
            case 2:
                s->makeItHome(sensor2);
                break;
            case 3:
                s->makeItHome(sensor1);
                break;
            }
        };
        // if (sensor)
        // {
        //     NetworkManager::getInstance().sendSystemLog("[MAIN] SENSOR FOUND");
        // }
        // if (sensor1)
        // {
        //     NetworkManager::getInstance().sendSystemLog("[MAIN] SENSOR1 FOUND");
        // }
        // if (sensor2)
        // {
        //     NetworkManager::getInstance().sendSystemLog("[MAIN] SENSOR2 FOUND");
        // }
    }

    if (pmpManager.isTimeout())
    {
        for (auto const &[id, servo] : servos.getServosMap())
        {
            ServoDevice *s = servos.getServo(id);
            if (s && s->isStatusSet(ServoDevice::ServoStatusFlags::ENABLED))
            {
                s->clearStatus(ServoDevice::ServoStatusFlags::ENABLED);
                s->setServoMode(99);
            };
        }
        NetworkManager::getInstance().sendSystemLog("[MAIN] Timeout komunikacji UDP! Serwa wyłączone");
    };

    ledsManager.update();

    if (!networkManager.isConnected())
    {
        ledsManager.setHeartbeat(true);
        ledsManager.setError(StatusLEDs::ErrorStatus::ETH_DISCONNECTED);
        for (auto const &[id, servo] : servos.getServosMap())
        {
            ServoDevice *s = servos.getServo(id);
            if (s && s->isStatusSet(ServoDevice::ServoStatusFlags::ENABLED))
            {
                s->clearStatus(ServoDevice::ServoStatusFlags::ENABLED);
                s->setServoMode(99);
            };
        }
        NetworkManager::getInstance().sendSystemLog("[MAIN] Kabel sieciowy odłączony!");
    }
    else
    {
        ledsManager.setHeartbeat(false);
        ledsManager.setError(StatusLEDs::ErrorStatus::IDLE);
    }
};