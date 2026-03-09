#include "CommandParser.h"

CommandManager::CommandManager(ServoControl &servoControl) : _servoControl(servoControl), _TuiManager(servoControl) {};

void CommandManager::dataParser(uint8_t *data, size_t size, AsyncClient *client)
{
    _TuiManager.init(client);
    String recievedCommand = "";
    for (size_t i = 0; i < size; i++)
    {
        recievedCommand += (char)data[i];
    }
    recievedCommand.trim();
    _TuiManager.handleCommand(recievedCommand, _servoControl);
};

TUIManager &CommandManager::getTuiManager()
{
    return _TuiManager;
};
