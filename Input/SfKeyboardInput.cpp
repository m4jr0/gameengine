/*******************************************************************************
 * Licence: http://creativecommons.org/licenses/by-nc-sa/3.0/
 * File: SfKeyboardInput.cpp
 * Author: m4jr0
 * Description:
 ******************************************************************************/

#include "SfKeyboardInput.h"

const sf::Keyboard::Key SfKeyboardInput::getKey() const {
    return this->key_;
}

SfKeyboardInput::SfKeyboardInput():
        SfKeyboardInput::SfKeyboardInput(sf::Keyboard::Unknown) {
}

SfKeyboardInput::SfKeyboardInput(sf::Keyboard::Key key) {
    this->key_ = key;
}

bool operator<(const SfKeyboardInput k1, const SfKeyboardInput k2) {
    return k1.getKey() < k2.getKey();
}