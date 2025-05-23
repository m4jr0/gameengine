// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "comet_pch.h"

#include "entry_point.h"

int main() {
  {
    auto engine{comet::GenerateEngine()};

    if (engine != nullptr) {
      engine->Populate();
    }
  }

  return EXIT_SUCCESS;
}