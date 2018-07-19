#ifndef GAME_HPP
#define GAME_HPP

#include <iostream>
#include <string>

#include "time_manager.hpp"
#include "input_manager.hpp"
#include "physics_manager.hpp"
#include "rendering_manager.hpp"
#include "game_object_manager.hpp"
#include "../utils/observer.hpp"

namespace koma {
class Game : public Observer {
public:
    const double MS_PER_UPDATE = 16.66;

    static koma::Game *Instance();

    virtual ~Game();
    Game(koma::Game const &) = delete;
    koma::Game &operator=(const Game &) = delete;
    void Run();
    void receive_event(std::string);

protected:
    Game();
    static koma::Game *instance;
    koma::PhysicsManager *physics_manager = nullptr;
    koma::RenderingManager *rendering_manager = nullptr;
    koma::InputManager *input_manager = nullptr;
    koma::TimeManager *time_manager = nullptr;
    koma::GameObjectManager *game_object_manager = nullptr;

private:
    bool is_running_ = false;
    int physics_frame_counter_ = 0;
    int rendering_frame_counter_ = 0;
};
}; // namespace koma

#endif // GAME_HPP
