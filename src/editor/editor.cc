// Copyright 2021 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "editor.h"

#ifdef _WIN32
// Allow debugging memory leaks on Windows.
#include "debug_windows.h"
#endif  // _WIN32

namespace comet {
namespace editor {
CometEditor::CometEditor() : core::Engine() {}

void CometEditor::Initialize() {
#ifdef _WIN32
  if (!SetConsoleCtrlHandler((PHANDLER_ROUTINE)HandleConsole, TRUE)) {
    std::cout
        << "Could not set console handler. This could result in "
        << "memory leaks as the engine would not shut down properly if the "
        << "console window is closed." << std::endl;
  }
#endif  // _WIN32

  core::Engine::Initialize();
}

void CometEditor::Exit() {
  core::Engine::Exit();

#ifdef _WIN32
  _CrtDumpMemoryLeaks();
#endif  // _WIN32
}

#ifdef _WIN32
BOOL WINAPI CometEditor::HandleConsole(DWORD window_event) {
  switch (window_event) {
    case CTRL_CLOSE_EVENT:
    case CTRL_LOGOFF_EVENT:
    case CTRL_SHUTDOWN_EVENT:
      CometEditor::engine()->Quit();
      return TRUE;

    default:
      break;
  }

  return FALSE;
}
#endif  // _WIN32
}  // namespace editor

std::unique_ptr<core::Engine> core::CreateEngine() {
  return std::make_unique<editor::CometEditor>();
}
}  // namespace comet