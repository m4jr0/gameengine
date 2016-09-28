/*******************************************************************************
 * Licence: http://creativecommons.org/licenses/by-nc-sa/3.0/
 * File: InputsComponent.h
 * Author: m4jr0
 * Description:
 ******************************************************************************/

#ifndef GAMEENGINE_INPUTSCOMPONENT_H
#define	GAMEENGINE_INPUTSCOMPONENT_H

#include "../Component/Component.h"

class InputsComponent : public Component {
public:
    InputsComponent();
    InputsComponent(GameObject*);
    InputsComponent(const InputsComponent&);
    virtual ~InputsComponent();
};

#endif //GAMEENGINE_INPUTSCOMPONENT_H