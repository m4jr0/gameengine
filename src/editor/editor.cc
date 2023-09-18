// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "editor.h"

#include "comet/core/type/tstring.h"
#include "comet/entity/factory/entity_factory_manager.h"

namespace comet {
namespace editor {
void CometEditor::Update(f64& lag) {
  Engine::Update(lag);
  camera_handler_->Update();
}

void CometEditor::PreLoad() {
  Engine::PreLoad();

#ifdef COMET_WINDOWS
  if (!SetConsoleCtrlHandler(static_cast<PHANDLER_ROUTINE>(HandleConsole),
                             TRUE)) {
    std::cout
        << "Could not set console handler. This could result in "
        << "memory leaks as the engine would not shut down properly if the "
        << "console window is closed." << '\n';
  }
#endif  // COMET_WINDOWS

#ifdef COMET_UNIX
  struct sigaction sig_handler;
  sig_handler.sa_handler = [](s32 signal) { CometEditor::Get().Quit(); };

  sigemptyset(&sig_handler.sa_mask);
  sig_handler.sa_flags = 0;

  sigaction(SIGINT, &sig_handler, NULL);
#endif  // COMET_UNIX

  asset_manager_ = std::make_unique<asset::AssetManager>();
  asset_manager_->Initialize();
}

// TODO(m4jr0): Remove temporary code.
void CometEditor::PostLoad() {
  Engine::PostLoad();
  PostLoadTmpCode();
}

void CometEditor::PostUnload() {
  PostUnloadTmpCode();
  asset_manager_->Shutdown();
  Engine::PostUnload();
}

#ifdef COMET_WINDOWS
BOOL WINAPI CometEditor::HandleConsole(DWORD window_event) {
  switch (window_event) {
    case CTRL_CLOSE_EVENT:
    case CTRL_LOGOFF_EVENT:
    case CTRL_SHUTDOWN_EVENT:
      CometEditor::Get().Quit();
      return TRUE;

    default:
      break;
  }

  return FALSE;
}
#endif  // COMET_WINDOWS

// TODO(m4jr0): Remove temporary code.
void CometEditor::PostLoadTmpCode() {
  entity::EntityFactoryManager::Get().GetModel()->Generate(
      COMET_CTSTRING_VIEW("models/nanosuit/model.obj"));
  camera_handler_ = std::make_unique<CameraHandler>();
  camera_handler_->Initialize();
}

void CometEditor::PostUnloadTmpCode() { camera_handler_->Shutdown(); }
}  // namespace editor

std::unique_ptr<Engine> GenerateEngine() {
  return std::make_unique<editor::CometEditor>();
}
}  // namespace comet