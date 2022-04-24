// Copyright 2022 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "entry_point.h"

int main(int argc, char* argv[]) {
  {
    auto engine{comet::CreateEngine()};

    if (engine != nullptr) {
      engine->Initialize();
      engine->Run();
      engine->Destroy();
    }
  }

  return EXIT_SUCCESS;
}