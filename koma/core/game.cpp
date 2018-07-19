#include "game.hpp"

koma::Game *koma::Game::instance = nullptr;

koma::Game::Game() {
    this->physics_manager = new koma::PhysicsManager();
    this->input_manager = new koma::InputManager();
    this->rendering_manager = new koma::RenderingManager();
    this->time_manager = new koma::TimeManager();
    this->game_object_manager = koma::GameObjectManager::Instance();

    this->time_manager->add_observer(this);
}

koma::Game::~Game() {
    delete this->physics_manager;
    delete this->input_manager;
    delete this->rendering_manager;
    delete this->time_manager;
    delete this->game_object_manager;
}

void koma::Game::Run() {
    this->is_running_ = true;
    this->time_manager->Initialize();
    double lag = 0.0;

    while (this->is_running_) {
        this->time_manager->Update();

        lag += this->time_manager->GetTimeDelta();
        this->input_manager->GetInputs();

        while (lag >= koma::Game::MS_PER_UPDATE) {
            this->physics_manager->Update();
            this->physics_frame_counter_++;
            lag -= koma::Game::MS_PER_UPDATE;
        }

        this->rendering_manager->Update(lag / koma::Game::MS_PER_UPDATE);
        this->rendering_frame_counter_++;
    }
}

koma::Game *koma::Game::Instance() {
    if (!koma::Game::instance) {
        koma::Game::instance = new koma::Game();
    }

    return koma::Game::instance;
}

void koma::Game::receive_event(std::string event) {
    std::cout << "PHYSICS " << this->physics_frame_counter_ << std::endl;
    std::cout << "RENDERING " << this->rendering_frame_counter_ << std::endl;

    this->physics_frame_counter_ = 0;
    this->rendering_frame_counter_ = 0;
}
