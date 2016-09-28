/*******************************************************************************
 * Licence: http://creativecommons.org/licenses/by-nc-sa/3.0/
 * File: InputsComponent.h
 * Author: KeyboardInputHandler
 * Description:
 ******************************************************************************/

#include <SFML/Window/Event.hpp>
#include "KeyboardInputHandler.h"

KeyboardInputHandler::~KeyboardInputHandler() {

}

bool KeyboardInputHandler::isPressed(Input* input) {
    if (SfKeyboardInput* key = dynamic_cast<SfKeyboardInput*>(input)) {
        return sf::Keyboard::isKeyPressed(key->getKey());
    }

    return false;
}

Command* KeyboardInputHandler::handleInput() {
    return nullptr;
}

Command* KeyboardInputHandler::handleInput(sf::Event event) {
    if (event.type == sf::Event::EventType::KeyPressed
            || event.type == sf::Event::EventType::KeyReleased) {
        return this->inputMap.getCommand(SfKeyboardInput(event.key.code));
    }

    return nullptr;
}

KeyboardInputHandler KeyboardInputHandler::createKeyboardInputHandler() {
    KeyboardInputHandler keyboardInputHandler = KeyboardInputHandler();
    keyboardInputHandler.inputMap.addCommand(
            SfKeyboardInput(sf::Keyboard::Key::Q),
            new BasicCommand()
    );

    return keyboardInputHandler;
}