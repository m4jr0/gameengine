/*******************************************************************************
 * Licence: http://creativecommons.org/licenses/by-nc-sa/3.0/
 * File: Command.h
 * Author: m4jr0
 * Description:
 ******************************************************************************/

#ifndef GAMEENGINE_COMMAND_H
#define GAMEENGINE_COMMAND_H

#include "../GameObject/GameActor.h"

class Command {
public:
    virtual ~Command();
    virtual void execute(GameActor&) = 0;
};


#endif //GAMEENGINE_COMMAND_H
