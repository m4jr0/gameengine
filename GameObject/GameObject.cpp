/*******************************************************************************
 * Licence: http://creativecommons.org/licenses/by-nc-sa/3.0/
 * File: GameObject.cpp
 * Author: m4jr0
 * Description:
 ******************************************************************************/

#include "GameObject.h"

GameObject::GameObject(): GameObject(
    new ComponentsContainer<PhysicsComponent>(),
    new ComponentsContainer<GraphicsComponent>(),
    new ComponentsContainer<InputsComponent>()
    ){
}

GameObject::GameObject
    (ComponentsContainer<PhysicsComponent>* physicsComponentsContainer, 
    ComponentsContainer<GraphicsComponent>* graphicsComponentsContainer, 
    ComponentsContainer<InputsComponent>* inputsComponentsContainer) {
        
    this->velocity_ = 0.0;
    this->position_ = new Position(0, 0);
    this->physicsComponentsContainer_ = physicsComponentsContainer;
    this->graphicsComponentsContainer_ = graphicsComponentsContainer;
    this->inputsComponentsContainer_ = inputsComponentsContainer;
}

GameObject::GameObject(const GameObject& orig) {
    this->velocity_ = orig.getVelocity();
    this->position_ = new Position(*orig.getPosition());
    GameObject* gTemp = const_cast<GameObject*>(&orig);
    this->physicsComponentsContainer_ = 
        new ComponentsContainer<PhysicsComponent>
        (*gTemp->getPhysicsComponentsContainer());
    this->graphicsComponentsContainer_ = 
        new ComponentsContainer<GraphicsComponent>
        (*gTemp->getGraphicsComponentsContainer());
    this->inputsComponentsContainer_ = 
        new ComponentsContainer<InputsComponent>
        (*gTemp->getInputsComponentsContainer());
}

GameObject::~GameObject() {
    delete this->position_;
    delete this->physicsComponentsContainer_;
    delete this->graphicsComponentsContainer_;
    delete this->inputsComponentsContainer_;
}

void GameObject::setPhysicsComponentsContainer
    (ComponentsContainer<PhysicsComponent>* physicsComponentsContainer) {
    delete this->physicsComponentsContainer_;
    this->physicsComponentsContainer_ = physicsComponentsContainer;
}

ComponentsContainer<PhysicsComponent>* GameObject::
    getPhysicsComponentsContainer() {
    return this->physicsComponentsContainer_;
}

void GameObject::setGraphicsComponentsContainer
    (ComponentsContainer<GraphicsComponent>* graphicsComponentsContainer) {
    delete this->graphicsComponentsContainer_;
    this->graphicsComponentsContainer_ = graphicsComponentsContainer;
}

ComponentsContainer<GraphicsComponent>* GameObject::
    getGraphicsComponentsContainer() {
    return this->graphicsComponentsContainer_;
}

void GameObject::setInputsComponentsContainer
    (ComponentsContainer<InputsComponent>* inputsComponentsContainer) {
    delete this->inputsComponentsContainer_;
    this->inputsComponentsContainer_ = inputsComponentsContainer;
}

ComponentsContainer<InputsComponent>* GameObject::
    getInputsComponentsContainer() {
    return this->inputsComponentsContainer_;
}

const double& GameObject::getVelocity() const {
    return this->velocity_;
}

void GameObject::setVelocity(double velocity) {
    this->velocity_ = velocity;
}

const Position* GameObject::getPosition() const {
    return this->position_;
}

void GameObject::setPosition(Position* position) {
    delete this->position_;
    this->position_ = position;
}

void GameObject::send(void* message) {
    this->physicsComponentsContainer_->send(message);
    this->graphicsComponentsContainer_->send(message);
    this->inputsComponentsContainer_->send(message);
}