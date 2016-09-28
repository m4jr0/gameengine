/*******************************************************************************
 * Licence: http://creativecommons.org/licenses/by-nc-sa/3.0/
 * File: InputHandler.h
 * Author: m4jr0
 * Description:
 ******************************************************************************/

#ifndef GAMEENGINE_INPUTHANDLER_H
#define GAMEENGINE_INPUTHANDLER_H


#include <map>
#include "Input.h"
#include "../Command/Command.h"

class InputHandler {
public:
    virtual ~InputHandler();
    virtual bool isPressed(Input* input) = 0;
    virtual Command* handleInput() = 0;
};


#endif //GAMEENGINE_INPUTHANDLER_H