// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "entry_point.h"

int main() {
  {
    auto engine{comet::GenerateEngine()};

    if (engine != nullptr) {
      engine->Initialize();
      engine->Run();
      engine->Shutdown();
    }
  }

  return EXIT_SUCCESS;
}