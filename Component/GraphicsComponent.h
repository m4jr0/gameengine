/*******************************************************************************
 * Licence: http://creativecommons.org/licenses/by-nc-sa/3.0/
 * File: GraphicsComponent.h
 * Author: m4jr0
 * Description:
 ******************************************************************************/

#ifndef GAMEENGINE_GRAPHICSCOMPONENT_H
#define	GAMEENGINE_GRAPHICSCOMPONENT_H

#include "Graphics.h"
#include "Component.h"

class GraphicsComponent : public Component {
public:
    GraphicsComponent();
    GraphicsComponent(GameObject*, Graphics*);
    GraphicsComponent(const GraphicsComponent&);
    virtual ~GraphicsComponent();
    const Graphics* getGraphics() const;
    void setGraphics(Graphics*);
private:
    Graphics* graphics_;
};

#endif //GAMEENGINE_GRAPHICSCOMPONENT_H