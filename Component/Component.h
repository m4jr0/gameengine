/*******************************************************************************
 * Licence: http://creativecommons.org/licenses/by-nc-sa/3.0/
 * File: Component.h
 * Author: m4jr0
 * Description:
 ******************************************************************************/

#ifndef GAMEENGINE_COMPONENT_H
#define	GAMEENGINE_COMPONENT_H

class GameObject;

class Component {
public:
    Component();
    Component(GameObject*);
    Component(const Component&);
    virtual ~Component();
    const GameObject* getGameObject() const;
    void setGameObject(GameObject*);
    virtual void receive(void*) = 0;
private:
    GameObject* gameObject_;
};

#endif	//GAMEENGINE_COMPONENT_H