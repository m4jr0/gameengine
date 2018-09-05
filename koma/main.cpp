// Copyright 2018 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#include <memory>

#ifdef _WIN32
#include <iostream>
#include <windows.h>
#endif  // _WIN32

#include "core/game.hpp"

// Allow debugging memory leaks.
#include "debug.hpp"

std::shared_ptr<koma::Game> game = nullptr;

BOOL WINAPI ConsoleHandler(DWORD);
void KillGame();

#ifdef _WIN32
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

void KillGame() {
  if (!game) return;

  game->Destroy();
}

int main(int argc, char *argv[]) {
  game = std::make_shared<koma::Game>();

#ifdef _WIN32
  if (!SetConsoleCtrlHandler(ConsoleHandler, TRUE)) {
    std::cout << "Could not set console handler. This could result in " <<
      "memory leaks as the game would not shut down properly if the " <<
      "console window is closed." << std::endl;
  }
#endif  // _WIN32

  game->Initialize();
  game->Run();

  _CrtDumpMemoryLeaks();

  return EXIT_SUCCESS;
}
