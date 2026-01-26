#include "TUIManager.h"

TUIManager::TUIManager(ServoControl &servoControl) : _servoControl(servoControl) {};

void TUIManager::init(AsyncClient *client)
{
    _activeClient = client;
};

void TUIManager::handleCommand(const String &command, ServoControl &servoControl)
{
    switch (_currentState)
    {
    case WindowState::MAIN_MENU:
        Serial.println("[TuiManager] Main menu");
        _mainMenuHandler(command);
        break;

    case WindowState::SERVO_CONTROL:
        Serial.println("[TuiManager] ServoControl");
        _servoControlHandler(command);
        break;

    case WindowState::NETWORK_STAT:
        _activeClient->write("Niedostępne. Wpisz [EXIT] żeby wyjść.");

        if (command == "EXIT")
        {
            _currentState = WindowState::MAIN_MENU;
        };
        break;

    case WindowState::PPCAN_SETTINGS:
        _activeClient->write("Niedostępne. Wpisz [EXIT] żeby wyjść.");

        if (command == "EXIT")
        {
            _currentState = WindowState::MAIN_MENU;
        };
        break;

    case WindowState::SERVO_MONITOR:
        // Serial.println("[TuiManager] Servo monitor");
        // if (_activeServo)
        // {
        //     _printServoControl();
        //     _servoControlHandler(command);
        // }
        // else if (!_activeServo)
        // {
        //     _activeServo = command.toInt();
        // };
        if (command == "e")
        {
            _activeClient->write("\x1B[?25h");
            _currentState = WindowState::MAIN_MENU;
            isMonitoring = false;
        };
        break;

    case WindowState::SERVO_SETTINGS:
        Serial.println("[TuiManager] Servo setting");
        _activeClient->write("Niedostępne. Wpisz [EXIT] żeby wyjść.");

        if (command == "EXIT")
        {
            _currentState = WindowState::MAIN_MENU;
        };
        break;

    case WindowState::SERVO_GO:
        Serial.println("[TuiManager] Servo go");
        if (_isAllDigit(command))
        {
            _servoControl.getServo(_activeServo)->position = command.toInt();
            //_currentState = WindowState::SERVO_CONTROL;
            _printServoGO();
        }
        else if (command == "EXIT")
        {
            _currentState = WindowState::SERVO_CONTROL;
            _printServoControl();
        }
        else
        {
            _activeClient->write("wracamy do menu, jak nie umiesz wpisać poprawnie\r\n");
            _currentState = WindowState::MAIN_MENU;
            _printMainMenu();
        }
        break;

    case WindowState::PARAM_SETTING:
        Serial.println("[TuiManager] PArameter setting");
        if (_isAllDigit(command))
        {
            if (_servoControl.getServo(_activeServo) != nullptr)
            {
                //_servoControl.getServo(_activeServo)->setParameters(_parameterToChange, command.toInt());
                if (_parameterToChange == ServoDevice::ParameterServo::SPEED)
                {
                    _servoControl.getServo(_activeServo)->speed = command.toInt();
                }
                else if (_parameterToChange == ServoDevice::ParameterServo::ACCELERATION)
                {
                    _servoControl.getServo(_activeServo)->acceleration = command.toInt();
                };
                _servoControl.saveServoState(_activeServo);
                _currentState = WindowState::SERVO_CONTROL;
                _printServoControl();
            }
            else
            {
                _activeClient->write("Podane serwo nie istnieje\r\n");
                _currentState = WindowState::MAIN_MENU;
                _printMainMenu();
            }
        }
        else
        {
            _activeClient->write("Spróbuj jeszcze raz ale tym razem wpisz same cyfry durniu: ");
        };
        break;

    case WindowState::SERVO_AWAIT_ID:
        if (_isAllDigit(command))
        {
            int servoId = command.toInt();
            if (servoControl.getServo(servoId) != nullptr)
            {
                _activeServo = servoId;
                _activeClient->write("servo wybrane! \r\n");
                _currentState = WindowState::SERVO_CONTROL;
                _printServoControl();
            }
            else
            {
                _activeClient->write("Podane serwo nie istnieje\r\n");
                _currentState = WindowState::MAIN_MENU;
                _printMainMenu();
            }
        }
        else
        {
            _activeClient->write("Podane ID nie jest liczbą -.-\r\n");
            _currentState = WindowState::MAIN_MENU;
            _printMainMenu();
        }
        break;

    case WindowState::ADD_SERWO:
        Serial.println("[TuiManager] Dodaj serwo");
        if (_isAllDigit(command))
        {
            int servoId = command.toInt();
            _servoControl.setServo(servoId);
            _activeClient->write("serwo dodane!\r\n");
            _currentState = WindowState::MAIN_MENU;
            _printMainMenu();
        }
        else
        {
            _activeClient->write("Podane ID nie jest liczbą spróbuj ponownie \r\n");
        }
        break;
    case WindowState::NETWORK_MONITOR:
        if (command == "EXIT")
        {
            _currentState = WindowState::MAIN_MENU;
            _printMainMenu();
        }
        break;
    case WindowState::DELETE_SERVO:
    {
        if (_isAllDigit(command))
        {
            if (_servoControl.deleteServo(command.toInt()))
            {
                _activeClient->write("------------- Serwo usunięto pomyślnie --------------\r\n");
            }
            else
            {
                _activeClient->write("!------------ [ERR] Serwo nie zostało usunięte -------------!\r\n");
            }
            _currentState = WindowState::MAIN_MENU;
            _printMainMenu();
        }
        else
        {
            _activeClient->write("To nie liczba, jeszcze raz spróbuj. Na pewno dasz radę.\r\n Spróbuj klawiszy z drugiego rzędu na klawiaturze.");
        };
    }
    break;
    case WindowState::TESTO:
        if (_isAllDigit(command))
        {
            _servoControl.getServo(_activeServo)->setServoMode(command.toInt());
            _currentState = WindowState::SERVO_CONTROL;
        }
    default:
        break;
    }
};

void TUIManager::onNewClientConnect(AsyncClient *client)
{
    _activeClient = client;

    _currentState = WindowState::MAIN_MENU;
    String clientIP = client->remoteIP().toString();
    String IPMsg = "Użytkownik o IP " + clientIP + " jest debilem.\r\n";

    _activeClient->write("===============================\r\n");
    _activeClient->write("O          c==================3\r\n");
    _activeClient->write("===============================\r\n");
    _activeClient->write(IPMsg.c_str());
    _printMainMenu();
};

// #################### MAIN MENU ##########################

// ekran
void TUIManager::_printMainMenu()
{

    _activeClient->write("===================MAIN MENU====================\r\n");
    _activeClient->write(_servoControl.listServoIDs().c_str());
    _activeClient->write("\r\n");
    _activeClient->write("Aktywne serwo: ");
    _activeClient->write(String(_activeServo).c_str());
    _activeClient->write(".\r\n");
    _activeClient->write("Wpisz liczbę do otrzymania odpowiedniej komendy:\r\n");
    _activeClient->write("[0] Ustawienia POLPAKEX \r\n");
    _activeClient->write("[1] Ustawienia Serw\r\n");
    _activeClient->write("[2] Kontrola Serwa\r\n");
    _activeClient->write("[3] Serwo INFO\r\n");
    _activeClient->write("[4] Wybór serwa\r\n");
    _activeClient->write("[5] Dodaj serwo\r\n");
    _activeClient->write("[6] Usuń serwo\r\n");
    _activeClient->write("[7] Monitoring komunikacji\r\n");
    _activeClient->write("[8] Monitoring towjej starej\r\n");
    _activeClient->write("================================================\r\n");
};

// obsługa
void TUIManager::_mainMenuHandler(const String &c)
{
    if (!_isAllDigit(c))
        return;
    switch (c.toInt())
    {
    case 0:
        //_currentState = WindowState::PPCAN_SETTINGS;
        _activeClient->write("wybierz coś innego, jeszcze nie skończone\r\n");
        break;

    case 1:
        _currentState = WindowState::SERVO_SETTINGS;
        break;

    case 2:
        _printServoControl();
        _currentState = WindowState::SERVO_CONTROL;
        break;

    case 3:
        if (_activeServo && _servoControl.getServo(_activeServo) != nullptr)
        {
            // for (int i = 0; i < 7; i++)
            // {
            //     ServoDevice::RealParameter param = ServoDevice::toRealParameter(i);
            //     int value = _servoControl.getServo(_activeServo)->getParameters(param);
            //     _printServoMonitor(value, param);
            // }
            _activeClient->write("\x1B[?25l");
            _activeClient->write("\r\n--- MONITORING SERWA ---");
            _activeClient->write("\r\nNaciśnij [e] aby wyjść \r\n");
            isMonitoring = true;
            _currentState = WindowState::SERVO_MONITOR;
        }
        else
        {
            String txt = "ID serwa: " + _activeServo;
            _activeClient->write("Błędne ID serwa \r\n");
            _activeClient->write(txt.c_str());
            _activeClient->write("\r\n");
        };

        _currentState = WindowState::MAIN_MENU;
        break;

    case 4:
        _activeClient->write("Wpisz ID serwa: ");
        _currentState = WindowState::SERVO_AWAIT_ID;
        break;

    case 5:
        _currentState = WindowState::ADD_SERWO;
        _activeClient->write("Wpisz ID nowego serwa: ");
        break;

    case 6:
        _currentState = WindowState::DELETE_SERVO;
        _activeClient->write("Wpisz ID serwa do usunięcia: ");
        break;

    case 7:
        _activeClient->write("Wpisz [EXIT] żeby wyjść.");
        _currentState = WindowState::NETWORK_MONITOR;
        break;
    case 8:
        _currentState = WindowState::UR_MAMA_MONITOR;
        break;
    default:
        _activeClient->write("coś Ty tu napisał za bzdury? jeszcze raz.");
        break;
    }
}
// ################################################################

// #################### KONTROLA SERWA ##########################
// ekran
void TUIManager::_printServoControl()
{
    _activeClient->write("=================SERVO CONTROL==================\r\n");
    _activeClient->write(_servoControl.listServoIDs().c_str());
    _activeClient->write("\r\n");
    _activeClient->write("Aktywne serwo: ");
    _activeClient->write(String(_activeServo).c_str());
    _activeClient->write(".\r\n");
    _activeClient->write("Wpisz liczbę do otrzymania odpowiedniej komendy:\r\n");
    _activeClient->write("[0] Włącz serwo \r\n");
    _activeClient->write("[1] Ustaw przyśpieszenie\r\n");
    _activeClient->write("[2] Ustaw prędkość\r\n");
    _activeClient->write("[3] Wyzeruj pozycje\r\n");
    _activeClient->write("[4] Wyłącz serwo\r\n");
    _activeClient->write("[GO] Jedź na pozycje\r\n");
    _activeClient->write("[EXIT] Wyjście\r\n");
    _activeClient->write("================================================\r\n");
    Serial.print(_activeServo);
};

// obsługa
void TUIManager::_servoControlHandler(const String &c)
{

    if (c == "0")
    {
        if (_servoControl.getServo(_activeServo) != nullptr)
        {
            _servoControl.getServo(_activeServo)->setServoMode(6);
            _activeClient->write("Servo załączone. \r\n");
            _printServoControl();
        }
        else
        {
            _activeClient->write("Nie ma takiego serwa!");
            _currentState = WindowState::MAIN_MENU;
            _printMainMenu();
        }
    }
    else if (c == "1" || c == "2")
    {
        if (c == "1")
        {
            _activeClient->write("Wpisz wartość przyśpieszenia: ");
            _parameterToChange = ServoDevice::ParameterServo::ACCELERATION;
        }
        else
        {
            _activeClient->write("Wpisz wartość prędkości: ");
            _parameterToChange = ServoDevice::ParameterServo::SPEED;
        }
        _currentState = WindowState::PARAM_SETTING;
    }
    else if (c == "3")
    {
        _servoControl.getServo(_activeServo)->setServoMode(6); //
        _activeClient->write("Servo załączone. \r\n");
        _printServoControl();
        _servoControl.getServo(_activeServo)->setZero();
    }
    else if (c == "4")
    {
        _servoControl.getServo(_activeServo)->setServoMode(99); //
    }
    else if (c == "TEST")
    {

        _activeClient->write("Wpisz tryb serwa: \r\n");
        _currentState = WindowState::TESTO;
    }

    else if (c == "GO")
    {
        _printServoGO();
    }
    else if (c == "EXIT")
    {
        _printMainMenu();
        _currentState = WindowState::MAIN_MENU;
        //_activeServo = 0;
    }
    else
    {
        _activeClient->write("eeeech, jeszcze raz wpisz.\r\n");
    };
};

// ruch serwa
void TUIManager::_printServoGO()
{
    if (_servoControl.getServo(_activeServo)->servoInPosition)
    {
        _activeClient->write("wpisz pozycje lub [EXIT] jeśli chcesz wyjść: ");
        _currentState = WindowState::SERVO_GO;
    }
    else
    {
        _activeClient->write("It's rolling........\r\n");
        _printServoControl();
    };
};

// ################################################################

// ###################### SERWO MONITOR #######################

void TUIManager::_printServoMonitor(int value, ServoDevice::RealParameter param)
{
    if (!_activeClient || !_activeClient->connected())
    {
        return;
    };
    switch (param)
    {
    case ServoDevice::RealParameter::SERVO_ID:
        _activeClient->write("--> ID: ");
        _activeClient->write(String(value).c_str());
        _activeClient->write(" | ");
        break;
    case ServoDevice::RealParameter::SERVO_MODE:
        _activeClient->write("Mode: ");
        _activeClient->write(String(value).c_str());
        _activeClient->write(" | ");
        break;
    case ServoDevice::RealParameter::POSITION:
        _activeClient->write("Pozycja: ");
        _activeClient->write(String(value).c_str());
        _activeClient->write(" | ");
        break;
    case ServoDevice::RealParameter::SPEED:
        _activeClient->write("Prędkość: ");
        _activeClient->write(String(value).c_str());
        _activeClient->write(" | ");
        break;
    case ServoDevice::RealParameter::CURRENT:
        _activeClient->write("Natężenie: ");
        _activeClient->write(String(value).c_str());
        _activeClient->write(" | ");
        break;
    case ServoDevice::RealParameter::ERROR:
        _activeClient->write("Error: ");
        _activeClient->write(String(value).c_str());
        _activeClient->write(" | ");
        break;
    case ServoDevice::RealParameter::TEMPERATURE:
        _activeClient->write("Temperatura: ");
        _activeClient->write(String(value).c_str());
        _activeClient->write(" | ");
        break;
    }
    //_activeClient->write("\r\n");
}
// ###################### MONITOR #####################

void TUIManager::printNetworkMonitor(const String &msg)
{
    if (_currentState == WindowState::NETWORK_MONITOR && _activeClient)
    {
        _activeClient->write(msg.c_str());
        _activeClient->write("\r\n");
    }
};

void TUIManager::printServoMonitor()
{
    _activeClient->write("\r");
    for (int i = 0; i < 7; i++)
    {
        ServoDevice::RealParameter param = ServoDevice::toRealParameter(i);
        int value = _servoControl.getServo(_activeServo)->getParameters(param);
        _printServoMonitor(value, param);
    };
    ServoDevice *servo = _servoControl.getServo(_activeServo);
    _activeClient->write("Enable: ");
    if (servo->isStatusSet(ServoDevice::ServoStatusFlags::ENABLED))
    {

        _activeClient->write("o");
    }
    else
    {
        _activeClient->write("x");
    };
    _activeClient->write(" | ");
    _activeClient->write("Online: ");
    if (servo->isStatusSet(ServoDevice::ServoStatusFlags::ONLINE))
    {
        _activeClient->write("o");
    }
    else
    {
        _activeClient->write("x");
    }
    _activeClient->write("In Position: ");
    if (servo->isStatusSet(ServoDevice::ServoStatusFlags::IN_POSITION))
    {

        _activeClient->write("o");
    }
    else
    {
        _activeClient->write("x");
    };
    _activeClient->write(" | ");
    _activeClient->write("Busy: ");
    if (servo->isStatusSet(ServoDevice::ServoStatusFlags::BUSY))
    {

        _activeClient->write("o");
    }
    else
    {
        _activeClient->write("x");
    };
    _activeClient->write(" | ");
    _activeClient->write("\x1B[K");
}

// ###################### FUNKCJE POMOCNICZE #######################

bool TUIManager::_isAllDigit(const String &str)
{
    if (str.length() == 0)
    {
        return false;
    };

    for (int i = 0; i < str.length(); i++)
    {

        if (!isdigit(str.charAt(i)))
        {
            return false;
        };
    };

    return true;
}