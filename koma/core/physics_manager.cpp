#include "physics_manager.hpp"

void koma::PhysicsManager::Update() {
    koma::GameObjectManager *game_object_manager = koma::GameObjectManager::Instance();
    game_object_manager->FixedUpdate();
}
