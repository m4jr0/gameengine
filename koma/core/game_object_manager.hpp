#ifndef GAME_OBJECT_MANAGER_HPP
#define GAME_OBJECT_MANAGER_HPP

#include <boost/functional/hash.hpp>
#include <boost/uuid/uuid.hpp>
#include <functional>
#include <unordered_map>

#include "../game_object/game_object.hpp"

namespace koma {
class GameObjectManager {
public:
    static koma::GameObjectManager *Instance();

    virtual ~GameObjectManager();
    GameObjectManager(koma::GameObjectManager const &) = delete;
    koma::GameObjectManager &operator=(const GameObjectManager &) = delete;
    void Update(double);
    void FixedUpdate();
    void AddGameObject(koma::GameObject*);
    void RemoveGameObject(koma::GameObject*);

protected:
    GameObjectManager();
    static koma::GameObjectManager *instance;

private:
    std::unordered_map<std::string, koma::GameObject*> game_objects_;
};
}; // namespace koma

#endif // GAME_OBJECT_MANAGER_HPP
