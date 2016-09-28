/*******************************************************************************
 * Licence: http://creativecommons.org/licenses/by-nc-sa/3.0/
 * File: Component.cpp
 * Author: m4jr0
 * Description:
 ******************************************************************************/

#include "Component.h"

Component::Component(): Component(nullptr) {
}

Component::Component(GameObject* gameObject) {
    this->gameObject_ = gameObject;
}

Component::Component(const Component& orig) {
    this->gameObject_ = const_cast<GameObject*>(orig.getGameObject());
}

Component::~Component() {
}
    
const GameObject* Component::getGameObject() const {
    return this->gameObject_;
}

void Component::setGameObject(GameObject* gameObject) {
    delete this->gameObject_;
    this->gameObject_ = gameObject;
}