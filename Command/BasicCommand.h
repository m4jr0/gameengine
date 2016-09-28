/*******************************************************************************
 * Licence: http://creativecommons.org/licenses/by-nc-sa/3.0/
 * File: BasicCommand.h
 * Author: m4jr0
 * Description:
 ******************************************************************************/

#ifndef GAMEENGINE_BASICCOMMAND_H
#define GAMEENGINE_BASICCOMMAND_H


#include "Command.h"

class BasicCommand: public Command {
public:
    virtual void execute(GameActor&);
};


#endif //GAMEENGINE_BASICCOMMAND_H
