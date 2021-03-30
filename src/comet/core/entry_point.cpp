// Copyright 2021 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "entry_point.hpp"

#ifdef _WIN32
// Allow debugging memory leaks on Windows.
#include "debug_windows.hpp"
#endif  // _WIN32

int main(int argc, char* argv[]) {
  auto engine = comet::CreateEngine();
  engine->Initialize();
  engine->Run();

  return EXIT_SUCCESS;
}
