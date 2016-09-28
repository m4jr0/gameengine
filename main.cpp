/*******************************************************************************
 * Licence: http://creativecommons.org/licenses/by-nc-sa/3.0/
 * File: main.cpp
 * Author: m4jr0
 * Description:
 ******************************************************************************/

#include "GameObject/GameObject.h"
#include "Component/PlayerInputsComponent.h"
#include "Component/PlayerPhysicsComponent.h"
#include "Component/PlayerGraphicsComponent.h"

GameObject* createPlayer(World*, Graphics*);

int main(int argc, char** argv) {
    World* world = new World();
    Graphics* graphics = new Graphics();
    GameObject* player = createPlayer(world, graphics);
    
    player->send(world);
    world->loop();

    delete world;
    delete graphics;
    delete player;
    
    return 0;
}

GameObject* createPlayer(World* world, Graphics* graphics) {
    GameObject* gameObject = new GameObject();
    ComponentsContainer<PhysicsComponent>* physicsComponentsContainer
        = new ComponentsContainer<PhysicsComponent>();
    ComponentsContainer<GraphicsComponent>* graphicsComponentsContainer
        = new ComponentsContainer<GraphicsComponent>();
    ComponentsContainer<InputsComponent>* inputsComponentsContainer
        = new ComponentsContainer<InputsComponent>();
    
    physicsComponentsContainer->addComponent(
        new PlayerPhysicsComponent(gameObject, world));
    graphicsComponentsContainer->addComponent(
        new PlayerGraphicsComponent(gameObject, graphics));
    inputsComponentsContainer->addComponent(
        new PlayerInputsComponent(gameObject));
    
    gameObject->setPhysicsComponentsContainer(physicsComponentsContainer);
    gameObject->setGraphicsComponentsContainer(graphicsComponentsContainer);
    gameObject->setInputsComponentsContainer(inputsComponentsContainer);
  
    return gameObject;
}