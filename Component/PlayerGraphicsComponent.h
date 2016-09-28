/*******************************************************************************
 * Licence: http://creativecommons.org/licenses/by-nc-sa/3.0/
 * File: PlayerGraphicsComponent.h
 * Author: m4jr0
 * Description:
 ******************************************************************************/

#ifndef GAMEENGINE_PLAYERGRAPHICSCOMPONENT_H
#define	GAMEENGINE_PLAYERGRAPHICSCOMPONENT_H

#include "GraphicsComponent.h"

class PlayerGraphicsComponent : public GraphicsComponent {
public:
    PlayerGraphicsComponent();
    PlayerGraphicsComponent(GameObject*, Graphics*);
    PlayerGraphicsComponent(const PlayerGraphicsComponent&);
    virtual ~PlayerGraphicsComponent();
    virtual void receive(void*);
private:
};

#endif //GAMEENGINE_PLAYERGRAPHICSCOMPONENT_H