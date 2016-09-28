/*******************************************************************************
 * Licence: http://creativecommons.org/licenses/by-nc-sa/3.0/
 * File: PlayerInputsComponent.h
 * Author: m4jr0
 * Description:
 ******************************************************************************/

#ifndef GAMEENGINE_PLAYERINPUTSCOMPONENT_H
#define	GAMEENGINE_PLAYERINPUTSCOMPONENT_H

#include "../Input/InputsComponent.h"

class PlayerInputsComponent : public InputsComponent {
public:
    PlayerInputsComponent();
    PlayerInputsComponent(GameObject*);
    PlayerInputsComponent(const PlayerInputsComponent&);
    virtual ~PlayerInputsComponent();
    virtual void receive(void*);
private:
};

#endif //GAMEENGINE_PLAYERINPUTSCOMPONENT_H