/*******************************************************************************
 * Licence: http://creativecommons.org/licenses/by-nc-sa/3.0/
 * File: ComponentsContainer.h
 * Author: m4jr0
 * Description:
 ******************************************************************************/

#ifndef GAMEENGINE_COMPONENTSCONTAINER_H
#define	GAMEENGINE_COMPONENTSCONTAINER_H

#include <vector>

#include "../GameObject/GameObject.h"

template<class T> class ComponentsContainer {
public:
    ComponentsContainer();
    ComponentsContainer(GameObject*, std::vector<T*>*);
    ComponentsContainer(const ComponentsContainer&);
    ~ComponentsContainer();
    const std::vector<T*>* getComponents() const;
    virtual void deleteComponents();
    virtual void addComponent(T*);
    virtual void removeComponent(int);
    virtual void removeComponent(T*);
    virtual void send(void*);
private:
    std::vector<T*>* components_;
    GameObject* gameObject_;
};

template <class T> ComponentsContainer<T>::ComponentsContainer() 
    : ComponentsContainer(nullptr, new std::vector<T*>()) {
}

template <class T> ComponentsContainer<T>::
    ComponentsContainer(GameObject* gameObject, std::vector<T*>* components) {
    this->gameObject_ = gameObject;
    this->components_ = components;    
}

template <class T> ComponentsContainer<T>::
    ComponentsContainer(const ComponentsContainer<T>& orig) {
    this->components_ 
            = const_cast<std::vector<T*>*>(orig.getComponents());    
}

template <class T> ComponentsContainer<T>::~ComponentsContainer() {    
    this->deleteComponents();
    delete this->components_;
}

template <class T> const std::vector<T*>* ComponentsContainer<T>::
    getComponents() const {
    return this->components_;
}

template <class T> void ComponentsContainer<T>::deleteComponents() {
    if (this->components_ != nullptr) {
        for(typename std::vector<T*>::iterator it = this->components_->begin(); 
                it != this->components_->end(); ++it) {
            delete *it;
        }
        
        this->components_->clear();
    }
}

template <class T> void ComponentsContainer<T>::addComponent(T* component) {
    this->components_->push_back(component);
}

template <class T> void ComponentsContainer<T>::removeComponent(int index) {
    typename std::vector<T*>::iterator begin = this->components_->begin();
    delete this->components_->at(index);
    this->components_->erase(begin + index);
}

template <class T> void ComponentsContainer<T>::
    removeComponent(T* componentToRemove) {
    typename std::vector<T*>::iterator begin = this->components_->begin();
    typename std::vector<T*>::iterator end = this->components_->end();
    typename std::vector<T*>::iterator removeIndex = 
            std::remove(begin, end, componentToRemove);
    delete *removeIndex;
    this->components_->erase(removeIndex);
}

template <class T> void ComponentsContainer<T>::send(void* message) {
    if (this->components_ != nullptr) {
        for (typename std::vector<T*>::iterator it = this->components_->begin(); 
                it != this->components_->end(); ++it) {
            (*it)->receive(message);
        }
    }
}

#endif //GAMEENGINE_COMPONENTSCONTAINER_H