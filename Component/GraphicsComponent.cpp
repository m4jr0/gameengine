/*******************************************************************************
 * Licence: http://creativecommons.org/licenses/by-nc-sa/3.0/
 * File: GraphicsComponent.cpp
 * Author: m4jr0
 * Description:
 ******************************************************************************/

#include "GraphicsComponent.h"

GraphicsComponent::GraphicsComponent(): GraphicsComponent(nullptr, nullptr) {
}


GraphicsComponent::GraphicsComponent(GameObject* gameObject, Graphics* graphics) 
    : Component(gameObject) {
    this->graphics_ = graphics;
}

GraphicsComponent::GraphicsComponent(const GraphicsComponent& orig) 
    : Component(orig) {
    this->graphics_ = const_cast<Graphics*>(orig.getGraphics());
}

GraphicsComponent::~GraphicsComponent() {
}

const Graphics* GraphicsComponent::getGraphics() const {
    return this->graphics_;
}

void GraphicsComponent::setGraphics(Graphics* graphics) {
    delete this->graphics_;
    this->graphics_ = graphics;
}