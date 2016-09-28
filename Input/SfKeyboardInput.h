/*******************************************************************************
 * Licence: http://creativecommons.org/licenses/by-nc-sa/3.0/
 * File: SfKeyboardInput.h
 * Author: m4jr0
 * Description:
 ******************************************************************************/

#ifndef GAMEENGINE_SFKEYBOARDINPUT_H
#define GAMEENGINE_SFKEYBOARDINPUT_H

#include <SFML/Window/Keyboard.hpp>
#include "Input.h"

class SfKeyboardInput: public Input {
public:
    const sf::Keyboard::Key getKey() const;
    SfKeyboardInput();
    SfKeyboardInput(sf::Keyboard::Key);
private:
    sf::Keyboard::Key key_;
};

bool operator<(const SfKeyboardInput, const SfKeyboardInput);

#endif //GAMEENGINE_SFKEYBOARDINPUT_H
