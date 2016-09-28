/*******************************************************************************
 * Licence: http://creativecommons.org/licenses/by-nc-sa/3.0/
 * File: PhysicsComponent.h
 * Author: m4jr0
 * Description:
 ******************************************************************************/

#ifndef GAMEENGINE_PHYSICSCOMPONENT_H
#define	GAMEENGINE_PHYSICSCOMPONENT_H

#include "../Game/World.h"
#include "Component.h"

class PhysicsComponent : public Component {
public:
    PhysicsComponent();
    PhysicsComponent(GameObject*, World*);
    PhysicsComponent(const PhysicsComponent&);
    virtual ~PhysicsComponent();
    const World* getWorld() const;
    void setWorld(World*);
private:
    World* world_;
};

#endif //GAMEENGINE_PHYSICSCOMPONENT_H