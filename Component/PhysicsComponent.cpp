/*******************************************************************************
 * Licence: http://creativecommons.org/licenses/by-nc-sa/3.0/
 * File: PhysicsComponent.cpp
 * Author: m4jr0
 * Description:
 ******************************************************************************/

#include "PhysicsComponent.h"

PhysicsComponent::PhysicsComponent(): PhysicsComponent(nullptr, nullptr) {
}

PhysicsComponent::PhysicsComponent(GameObject* gameObject, World* world) 
    : Component(gameObject) {
    this->world_ = world;
}

PhysicsComponent::PhysicsComponent(const PhysicsComponent& orig) 
    : Component(orig) {
    this->world_ = const_cast<World*>(orig.getWorld());
}

PhysicsComponent::~PhysicsComponent() {
}
const World* PhysicsComponent::getWorld() const {
    return this->world_;
}

void PhysicsComponent::setWorld(World* world) {
    delete this->world_;
    this->world_ = world;
}