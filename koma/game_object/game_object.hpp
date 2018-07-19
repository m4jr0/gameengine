#ifndef GAME_OBJECT_HPP
#define GAME_OBJECT_HPP

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <iostream>

namespace koma {
class GameObject {
public:
    virtual ~GameObject() {};
    virtual void Update(double);
    virtual void FixedUpdate();
    const boost::uuids::uuid GetId() const;

private:
    boost::uuids::uuid id_ = boost::uuids::random_generator()();
};
}; // namespace koma

#endif // GAME_OBJECT_HPP
