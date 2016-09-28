/*******************************************************************************
 * Licence: http://creativecommons.org/licenses/by-nc-sa/3.0/
 * File: World.cpp
 * Author: m4jr0
 * Description:
 ******************************************************************************/

#include <SFML/Window/Event.hpp>
#include <SFML/Window/VideoMode.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include "World.h"
#include "../Input/KeyboardInputHandler.h"

World::World(): World(this->DEFAULT_MS_PER_UPDATE) {
}

World::World(double msPerUpdate): World(this->DEFAULT_MS_PER_UPDATE,
                                       this->DEFAULT_FPS_CAP) {
}

World::World(double msPerUpdate, unsigned int fpsCap) {
    this->msPerUpdate_ = msPerUpdate;
    this->fpsCap_ = fpsCap;
    this->running_ = false;
}

World::World(const World& orig) {
    this->msPerUpdate_ = orig.getMsPerUpdate();
}

World::~World() {
}

const double World::getMsPerUpdate() const {
    return this->msPerUpdate_;
}

void World::setMsPerUpdate(double msPerUpdate) {
    this->msPerUpdate_ = msPerUpdate;
}

void World::loop() {
    time_t previous = time(0);
    double lag = 0.0;
    KeyboardInputHandler keyboardInputHandler =
    KeyboardInputHandler::createKeyboardInputHandler();

    this->running_ = true;

    sf::RenderWindow window(sf::VideoMode(200, 200), "World");
    window.setFramerateLimit(this->fpsCap_);

    while (this->running_ && window.isOpen()) {
        sf::Event event;

        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
        }

        double elapsed = (double(time(0) - previous));
        previous = time(0);
        lag += elapsed;

        Command *command = keyboardInputHandler.handleInput(event);

        if (command) {
            GameActor g = GameActor();
            command->execute(g);
        }

        while (lag >= this->msPerUpdate_) {
            // update();
            lag -= this->msPerUpdate_;
        }

        // render(lag / this->msPerUpdate_);

        window.clear();
        window.display();
    }
}

const unsigned int World::getFpsCap() const {
    return this->fpsCap_;
}

void World::setFpsCap(unsigned int fpsCap) {
    this->fpsCap_ = fpsCap;
}

