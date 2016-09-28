/*******************************************************************************
 * Licence: http://creativecommons.org/licenses/by-nc-sa/3.0/
 * File: GameState.h
 * Author: m4jr0
 * Description:
 ******************************************************************************/

#ifndef GAMEENGINE_GAMESTATE_H
#define	GAMEENGINE_GAMESTATE_H

#include "GameObject.h"
#include "../Input/Input.h"

class GameState {
public:
    GameState();
    GameState(const GameState&);
    virtual GameState* handleInput(GameObject&, Input&) = 0;
    virtual ~GameState();
};

#endif //GAMEENGINE_GAMESTATE_H