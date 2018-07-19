#include "game_object_manager.hpp"

koma::GameObjectManager *koma::GameObjectManager::instance = nullptr;

koma::GameObjectManager::GameObjectManager() {
    this->AddGameObject(new GameObject());
}

koma::GameObjectManager::~GameObjectManager() {
    for (auto it : this->game_objects_) {
        delete it.second;
    }
}

void koma::GameObjectManager::Update(double interpolation) {
    for (auto it : this->game_objects_) {
        it.second->Update(interpolation);
    }
}

void koma::GameObjectManager::FixedUpdate() {
    for (auto it : this->game_objects_) {
        it.second->FixedUpdate();
    }
}

void koma::GameObjectManager::AddGameObject(koma::GameObject *game_object) {
    this->game_objects_.insert({boost::uuids::to_string(game_object->GetId()), game_object});
}

void koma::GameObjectManager::RemoveGameObject(koma::GameObject *game_object) {
    this->game_objects_.erase(boost::uuids::to_string(game_object->GetId())); 
}

koma::GameObjectManager *koma::GameObjectManager::Instance() {
    if (!koma::GameObjectManager::instance) {
        koma::GameObjectManager::instance = new koma::GameObjectManager();
    }

    return koma::GameObjectManager::instance;
}
