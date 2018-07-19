#ifndef OBSERVER_HPP
#define OBSERVER_HPP

#include <string>

namespace koma {
class Observer {
public:
    virtual void receive_event(std::string) = 0;
};
}; // namespace koma

#endif // OBSERVER_HPP