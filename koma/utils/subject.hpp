#ifndef SUBJECT_HPP
#define SUBJECT_HPP

#include <string>
#include <unordered_set>

#include "observer.hpp"

namespace koma {
class Subject {
public:
    virtual void add_observer(Observer*);
    virtual void remove_observer(Observer*);
    virtual void notify_observers(std::string);

private:
    std::unordered_set<Observer*> observers_;
};
}; // namespace koma

#endif // SUBJECT_HPP