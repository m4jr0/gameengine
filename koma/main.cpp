// Copyright 2018 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

// Allows debugging memory leaks.
// TODO(m4jr0): Find a better solution.
#include "debug.hpp"

#include "core/game.hpp"

int main(int argc, char *argv[]) {
  koma::Game game = koma::Game();

  game.Initialize();
  game.Run();

  _CrtDumpMemoryLeaks();

  return EXIT_SUCCESS;
}
