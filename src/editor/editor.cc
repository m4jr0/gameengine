// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "editor.h"
////////////////////////////////////////////////////////////////////////////////

#ifdef COMET_MSVC
#include <iostream>

#include "comet/core/windows.h"
#else
#include <signal.h>
#endif  // COMET_MSVC

#include "comet/core/logger/logging.h"
#include "comet/core/memory/memory_label.h"
#include "editor/asset/asset_manager.h"
#include "editor/memory/memory_label.h"

namespace comet {
namespace editor {
void CometEditor::Update(f64& lag) {
  Engine::Update(lag);
  COMET_ASSERT(camera_handler_ != nullptr, "CometEditor::Update",
               "camera handler is null");
  camera_handler_->Update();
}

void CometEditor::OnPreLoadAfter() {
  COMET_ATTACH_CUSTOM_MEMORY_LABEL_FUNC(memory::GetEditorMemoryTagLabel);
}

void CometEditor::OnLoadBefore() {
#ifdef COMET_WINDOWS
  if (!SetConsoleCtrlHandler(static_cast<PHANDLER_ROUTINE>(HandleConsole),
                             TRUE)) {
    std::cerr
        << "CometEditor::OnLoadBefore: console handler registration failed"
        << '\n';
  }
#endif  // COMET_WINDOWS

#ifdef COMET_UNIX
  struct sigaction sig_handler;
  sig_handler.sa_handler = [](s32) { CometEditor::Get().Quit(); };

  sigemptyset(&sig_handler.sa_mask);
  sig_handler.sa_flags = 0;

  sigaction(SIGINT, &sig_handler, NULL);
#endif  // COMET_UNIX

  LoadTmpCode();
  auto& asset_manager{asset::AssetManager::Get()};
  asset_manager.Initialize();

  COMET_LOG_INFO(LoggerType::External, "CometEditor::OnLoadBefore",
                 "editor assets refresh started");
  asset_manager.Refresh();
  COMET_LOG_INFO(LoggerType::External, "CometEditor::OnLoadBefore",
                 "editor assets refresh completed");
}

// TODO(m4jr0): Remove temporary code.
void CometEditor::OnPostLoadAfter() {
  camera_handler_ = std::make_unique<CameraHandler>();
  COMET_ASSERT(camera_handler_ != nullptr, "CometEditor::OnPostLoadAfter",
               "camera handler allocation failed");
  camera_handler_->Initialize();
}

void CometEditor::OnPreUnloadBefore() {
  if (camera_handler_ != nullptr) {
    camera_handler_->Shutdown();
    camera_handler_ = nullptr;
  }
}

void CometEditor::OnPostUnloadBefore() {
  asset::AssetManager::Get().Shutdown();
}

void CometEditor::OnPostUnloadAfter() {
  COMET_DETACH_CUSTOM_MEMORY_LABEL_FUNC();
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
void CometEditor::LoadTmpCode() {
  // Insert pre-engine initialization code here.
  // This can include tasks like deleting specific resources to force them to be
  // re-exported.
}
}  // namespace editor

memory::UniquePtr<Engine> GenerateEngine() {
  return std::make_unique<editor::CometEditor>();
}
}  // namespace comet