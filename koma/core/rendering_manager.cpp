#include "rendering_manager.hpp"

void koma::RenderingManager::Update(double interpolation) {
    koma::GameObjectManager *game_object_manager = koma::GameObjectManager::Instance();
    game_object_manager->Update(interpolation);
}
