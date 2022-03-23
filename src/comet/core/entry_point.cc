// Copyright 2022 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "entry_point.h"

#ifdef _WIN32
#include "debug_windows.h"
#endif  // _WIN32

int main(int argc, char* argv[]) {
  auto engine = comet::core::CreateEngine();
  engine->Initialize();
  engine->Run();

  return EXIT_SUCCESS;
}
