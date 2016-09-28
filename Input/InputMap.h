/*******************************************************************************
 * Licence: http://creativecommons.org/licenses/by-nc-sa/3.0/
 * File: ComponentsContainer.h
 * Author: m4jr0
 * Description:
 ******************************************************************************/

#include <map>

#ifndef GAMEENGINE_INPUTMAP_H
#define GAMEENGINE_INPUTMAP_H

template<class KEY, class VALUE> class InputMap {
public:
    VALUE getCommand(KEY);
    void addCommand(KEY, VALUE);
    void removeCommand(KEY);
    void removeCommand(VALUE);
    std::map<KEY, VALUE> inputMap;
};

template<class KEY, class VALUE> VALUE InputMap<KEY, VALUE>::
        getCommand(KEY input) {
    typename std::map<KEY, VALUE>::iterator commandIt
            = this->inputMap.find(input);

    if (commandIt == this->inputMap.end()) {
        return nullptr;
    }

    return (*commandIt).second;
}

template<class KEY, class VALUE> void InputMap<KEY, VALUE>::
        addCommand(KEY input, VALUE command) {
    this->inputMap.insert(
            typename std::map<KEY, VALUE>::value_type(input, command)
    );
}

template<class KEY, class VALUE> void InputMap<KEY, VALUE>::
        removeCommand(KEY input) {
    this->inputMap.erase(input);
}

template<class KEY, class VALUE> void InputMap<KEY, VALUE>::
        removeCommand(VALUE command) {
    typename std::map<KEY, VALUE>::iterator commandIt;

    for (commandIt = this->inputMap.begin();
         commandIt != this->inputMap.end();
         ++commandIt) {
        if ((commandIt->second) == command) {
            this->inputMap.erase(commandIt);

            return;
        }
    }
}

#endif //GAMEENGINE_INPUTMAP_H