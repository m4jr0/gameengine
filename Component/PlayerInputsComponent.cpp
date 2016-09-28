/*******************************************************************************
 * Licence: http://creativecommons.org/licenses/by-nc-sa/3.0/
 * File: PlayerInputsComponent.cpp
 * Author: m4jr0
 * Description:
 ******************************************************************************/

#include "PlayerInputsComponent.h"

PlayerInputsComponent::PlayerInputsComponent() {
}

PlayerInputsComponent::PlayerInputsComponent(GameObject* gameObject) 
    : InputsComponent(gameObject) {    
}

PlayerInputsComponent::PlayerInputsComponent
    (const PlayerInputsComponent& orig) {
}

PlayerInputsComponent::~PlayerInputsComponent() {
}

void PlayerInputsComponent::receive(void* message) {
    // code
}