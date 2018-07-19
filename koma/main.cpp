#include <iostream>

#include "core/game.hpp"
#include "game_object/game_object.hpp"

int main(int argc, char *argv[]) {
    koma::Game *game = koma::Game::Instance();

    

    game->Run();
}
