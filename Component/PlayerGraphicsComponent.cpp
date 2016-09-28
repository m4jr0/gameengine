/*******************************************************************************
 * Licence: http://creativecommons.org/licenses/by-nc-sa/3.0/
 * File: PlayerGraphicsComponent.cpp
 * Author: m4jr0
 * Description:
 ******************************************************************************/

#include "PlayerGraphicsComponent.h"

PlayerGraphicsComponent::PlayerGraphicsComponent() {
}

PlayerGraphicsComponent::PlayerGraphicsComponent
    (GameObject* gameObject, Graphics* graphics) 
        : GraphicsComponent(gameObject, graphics) {    
}

PlayerGraphicsComponent::PlayerGraphicsComponent
    (const PlayerGraphicsComponent& orig) {
}

PlayerGraphicsComponent::~PlayerGraphicsComponent() {
}

void PlayerGraphicsComponent::receive(void* message) {
    // code
}