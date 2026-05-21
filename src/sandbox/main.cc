// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/engine/engine.h"
#include "sandbox/sandbox.h"

int main() {
  comet::sandbox::Sandbox sandbox{};
  comet::Engine engine{&sandbox};
  engine.Populate();
  return EXIT_SUCCESS;
}