#include "game_object.hpp"

void koma::GameObject::Update(double interpolation) {
    // std::cout << boost::uuids::to_string(this->id) << ": Update(double)" << std::endl;
}

void koma::GameObject::FixedUpdate() {
    // std::cout << boost::uuids::to_string(this->id) << ": FixedUpdate()" << std::endl;
}

const boost::uuids::uuid koma::GameObject::GetId() const {
    return this->id_;
}
