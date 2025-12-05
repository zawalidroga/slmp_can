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
        _mainMenuHandler(command);
        break;

    case WindowState::SERVO_CONTROL:
        _servoControlHandler(command);
        break;

    case WindowState::NETWORK_STAT:
        break;

    case WindowState::PPCAN_SETTINGS:
        break;

    case WindowState::SERVO_MONITOR:

        if (_activeServo)
        {
            _printServoControl();
            _servoControlHandler(command);
        }
        else if (!_activeServo)
        {
            _activeServo = command.toInt();
        };

        break;

    case WindowState::SERVO_SETTINGS:

        break;

    case WindowState::SERVO_GO:
        if (_isAllDigit(command))
        {

            _servoControl.getServo(_activeServo)->position = command.toInt();
            _currentState = WindowState::SERVO_CONTROL;
        }
        else
        {
            _activeClient->write("wracamy do menu, jak nie umiesz wpisać poprawnie");
            _currentState = WindowState::MAIN_MENU;
        }
        break;

    case WindowState::PARAM_SETTING:
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
                _currentState = WindowState::SERVO_CONTROL;
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
        if (_isAllDigit(command))
        {
            int servoId = command.toInt();
            _servoControl.setServo(servoId);
            _activeClient->write("serwo dodane!");
        }
        else
        {
            _activeClient->write("Podane ID nie jest liczbą spróbuj ponownie \r\n");
        }
        break;

    default:
        break;
    }
};

void TUIManager::onNewClientConnect(AsyncClient *client)
{
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
    String actualServoID = "Servo ID: " + _activeServo;
    _activeClient->write("===================MAIN MENU====================\r\n");
    _activeClient->write(_servoControl.listServoIDs().c_str());
    _activeClient->write(actualServoID.c_str());
    _activeClient->write("Wpisz liczbę do otrzymania odpowiedniej komendy:\r\n");
    _activeClient->write("[0] Ustawienia POLPAKEX \r\n");
    _activeClient->write("[1] Ustawienia Serw\r\n");
    _activeClient->write("[2] Kontrola Serwa\r\n");
    _activeClient->write("[3] Serwo INFO\r\n");
    _activeClient->write("[4] Wybór serwa\r\n");
    _activeClient->write("[5] Dodaj serwo\r\n");
    _activeClient->write("[6] Monitoring towjej starej\r\n");
    _activeClient->write("================================================\r\n");
};

// obsługa
void TUIManager::_mainMenuHandler(const String &c)
{
    if (_isAllDigit(c))
        return;
    switch (c.toInt())
    {
    case 0:
        //_currentState = WindowState::PPCAN_SETTINGS;
        _activeClient->write("wybierz coś innego, jeszcze nie skończone");
        break;

    case 1:
        _currentState = WindowState::SERVO_SETTINGS;
        break;

    case 2:
        _printServoControl();
        _currentState = WindowState::SERVO_CONTROL;
        break;

    case 3:
        for (int i; i < 7; i++)
        {
            ServoDevice::RealParameter param = ServoDevice::toRealParameter(i);
            int value = _servoControl.getServo(_activeServo)->getAllParameters(param);
            _printServoMonitor(value, param);
        }

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
        _currentState = WindowState::UR_MAMA_MONITOR;
        break;
    default:
        _activeClient->write("coś Ty tu napisał za bzdury? jeszcze raz.");
    }
}
// ################################################################

// #################### KONTROLA SERWA ##########################
// ekran
void TUIManager::_printServoControl()
{
    String actualServoID = "Servo ID: " + _activeServo;
    _activeClient->write("=================SERVO CONTROL==================\r\n");
    _activeClient->write(_servoControl.listServoIDs().c_str());
    _activeClient->write(actualServoID.c_str());
    _activeClient->write("Wpisz liczbę do otrzymania odpowiedniej komendy:\r\n");
    _activeClient->write("[0] Włącz serwo \r\n");
    _activeClient->write("[1] Ustaw przyśpieszenie\r\n");
    _activeClient->write("[2] Ustaw prędkość\r\n");
    _activeClient->write("[3] Wyzeruj pozycje\r\n");
    _activeClient->write("[GO] Jedź na pozycje\r\n");
    _activeClient->write("[EXIT] Wyjście\r\n");
    _activeClient->write("================================================\r\n");
};

// obsługa
void TUIManager::_servoControlHandler(const String &c)
{

    if (c == "0")
    {
        if (_servoControl.getServo(_activeServo) != nullptr)
        {
            _servoControl.getServo(_activeServo)->setServoMode(6);
            _activeClient->write("Servo załączone.");
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
        _servoControl.getServo(_activeServo)->setZero();
    }

    else if (c == "GO")
    {
        _printServoGO();
    }
    else if (c == "EXIT")
    {
        _printMainMenu();
        _activeServo = 0;
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
        _activeClient->write("wpisz pozycje: ");
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
    String txt;
    switch (param)
    {
    case ServoDevice::RealParameter::SERVO_ID:
        txt = "ID: " + value;
        _activeClient->write(txt.c_str());
        break;
    case ServoDevice::RealParameter::SERVO_MODE:
        txt = "Servo Mode: " + value;
        _activeClient->write(txt.c_str());
        break;
    case ServoDevice::RealParameter::POSITION:
        txt = "Aktualna pozycja: " + value;
        _activeClient->write(txt.c_str());
        break;
    case ServoDevice::RealParameter::SPEED:
        txt = "Aktualna prędkość: " + value;
        _activeClient->write(txt.c_str());
        break;
    case ServoDevice::RealParameter::CURRENT:
        txt = "Aktualne natężenie: " + value;
        _activeClient->write(txt.c_str());
        break;
    case ServoDevice::RealParameter::ERROR:
        txt = "Error: " + value;
        _activeClient->write(txt.c_str());
        break;
    case ServoDevice::RealParameter::TEMPERATURE:
        txt = "Temperatura: " + value;
        _activeClient->write(txt.c_str());
        break;
    }
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
        }
    }

    return true;
}