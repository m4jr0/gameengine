/*******************************************************************************
 * Licence: http://creativecommons.org/licenses/by-nc-sa/3.0/
 * File: InputsComponent.cpp
 * Author: m4jr0
 * Description:
 ******************************************************************************/

#include "InputsComponent.h"

InputsComponent::InputsComponent() {
}

InputsComponent::InputsComponent(GameObject* gameObject) 
    : Component(gameObject) {
}

InputsComponent::InputsComponent(const InputsComponent& orig) 
    : Component(orig) {
}

InputsComponent::~InputsComponent() {
}