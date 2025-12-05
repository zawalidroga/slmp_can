#ifndef COMMAND_PARSER_H
#define COMMAND_PARSER_H

#include <Arduino.h>
#include "../can/ServoControl.h"
#include "../gui/TUIManager.h"

class CommandManager
{
private:
    ServoControl &_servoControl;
    TUIManager _TuiManager;

public:
    CommandManager(ServoControl &servoControl);
    void dataParser(uint8_t *data, size_t size, AsyncClient *client);
    TUIManager &getTuiManager();
};

#endif