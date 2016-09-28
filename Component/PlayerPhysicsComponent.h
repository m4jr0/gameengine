/*******************************************************************************
 * Licence: http://creativecommons.org/licenses/by-nc-sa/3.0/
 * File: PlayerPhysicsComponent.h
 * Author: m4jr0
 * Description:
 ******************************************************************************/

#ifndef GAMEENGINE_PLAYERPHYSICSCOMPONENT_H
#define	GAMEENGINE_PLAYERPHYSICSCOMPONENT_H

#include "PhysicsComponent.h"

class PlayerPhysicsComponent : public PhysicsComponent {
public:
    PlayerPhysicsComponent();
    PlayerPhysicsComponent(GameObject*, World*);
    PlayerPhysicsComponent(const PlayerPhysicsComponent& orig);
    virtual ~PlayerPhysicsComponent();
    virtual void receive(void*);
private:
};

#endif //GAMEENGINE_PLAYERPHYSICSCOMPONENT_H