/*******************************************************************************
 * Licence: http://creativecommons.org/licenses/by-nc-sa/3.0/
 * File: PlayerPhysicsComponent.cpp
 * Author: m4jr0
 * Description:
 ******************************************************************************/

#include "PlayerPhysicsComponent.h"

PlayerPhysicsComponent::PlayerPhysicsComponent() {
}

PlayerPhysicsComponent::PlayerPhysicsComponent
    (GameObject* gameObject, World* world) 
        : PhysicsComponent(gameObject, world) {    
}

PlayerPhysicsComponent::PlayerPhysicsComponent
    (const PlayerPhysicsComponent& orig) {
}

PlayerPhysicsComponent::~PlayerPhysicsComponent() {
}

void PlayerPhysicsComponent::receive(void* message) {
    // code
}