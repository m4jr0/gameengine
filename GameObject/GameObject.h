/*******************************************************************************
 * Licence: http://creativecommons.org/licenses/by-nc-sa/3.0/
 * File: GameObject.h
 * Author: m4jr0
 * Description:
 ******************************************************************************/

#ifndef GAMEENGINE_GAMEOBJECT_H
#define	GAMEENGINE_GAMEOBJECT_H

#include <vector>
#include <algorithm>

#include "../Component/PhysicsComponent.h"
#include "../Component/GraphicsComponent.h"
#include "../Input/InputsComponent.h"
#include "../Component/ComponentsContainer.h"
#include "../Game/World.h"
#include "../Component/Graphics.h"
#include "../Utility/Position.h"

class GameObject {
public:
    GameObject();
    GameObject(ComponentsContainer<PhysicsComponent>*, 
        ComponentsContainer<GraphicsComponent>*, 
        ComponentsContainer<InputsComponent>*);
    GameObject(const GameObject&);
    virtual ~GameObject();
    void setPhysicsComponentsContainer(ComponentsContainer<PhysicsComponent>*);
    ComponentsContainer<PhysicsComponent>* getPhysicsComponentsContainer();
    void setGraphicsComponentsContainer
            (ComponentsContainer<GraphicsComponent>*);
    ComponentsContainer<GraphicsComponent>* getGraphicsComponentsContainer();
    void setInputsComponentsContainer(ComponentsContainer<InputsComponent>*);
    ComponentsContainer<InputsComponent>* getInputsComponentsContainer();
    const double& getVelocity() const;
    void setVelocity(double);
    const Position* getPosition() const;
    void setPosition(Position*);
    virtual void send(void*);
private:
    double velocity_;
    Position* position_;
    ComponentsContainer<PhysicsComponent>* physicsComponentsContainer_;
    ComponentsContainer<GraphicsComponent>* graphicsComponentsContainer_;
    ComponentsContainer<InputsComponent>* inputsComponentsContainer_;
};

#endif //GAMEENGINE_GAMEOBJECT_H