/*******************************************************************************
 * Licence: http://creativecommons.org/licenses/by-nc-sa/3.0/
 * File: InputsComponent.h
 * Author: KeyboardInputHandler
 * Description:
 ******************************************************************************/

#ifndef GAMEENGINE_KEYBOARDINPUTHANDLER_H
#define GAMEENGINE_KEYBOARDINPUTHANDLER_H

#include "InputHandler.h"
#include "InputMap.h"
#include "SfKeyboardInput.h"
#include "../Command/BasicCommand.h"

class KeyboardInputHandler: public InputHandler {
public:
    virtual ~KeyboardInputHandler();
    virtual bool isPressed(Input*);
    virtual Command* handleInput();
    virtual Command* handleInput(sf::Event);
    static KeyboardInputHandler createKeyboardInputHandler();
    InputMap<SfKeyboardInput, Command*> inputMap;
};

#endif //GAMEENGINE_KEYBOARDINPUTHANDLER_H
