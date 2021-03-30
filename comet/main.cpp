// Copyright 2021 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include <memory>

#include "core/game.hpp"

#ifdef _WIN32
#include <windows.h>

#include <iostream>

// Allow debugging memory leaks on Windows.
#include "debug_windows.hpp"
#endif  // _WIN32

comet::Game* game = nullptr;

void KillGame();

void KillGame() {
  if (game == nullptr) return;

  game->Destroy();
}

#ifdef _WIN32
BOOL WINAPI ConsoleHandler(DWORD);

BOOL WINAPI ConsoleHandler(DWORD dwType) {
  switch (dwType) {
    case CTRL_CLOSE_EVENT:
    case CTRL_LOGOFF_EVENT:
    case CTRL_SHUTDOWN_EVENT:
      KillGame();

      return TRUE;

    default:
      break;
  }

  return FALSE;
}
#endif  // _WIN32

int main(int argc, char* argv[]) {
  game = comet::Game::game();

#ifdef _WIN32
  if (!SetConsoleCtrlHandler(ConsoleHandler, TRUE)) {
    std::cout << "Could not set console handler. This could result in "
              << "memory leaks as the game would not shut down properly if the "
              << "console window is closed." << std::endl;
  }
#endif  // _WIN32

  game->Initialize();
  game->Run();

#ifdef _WIN32
  _CrtDumpMemoryLeaks();
#endif  // _WIN32

  return EXIT_SUCCESS;
}
