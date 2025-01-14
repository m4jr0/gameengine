// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "comet_pch.h"

#include "editor.h"

#ifdef COMET_MSVC
#include <iostream>

#include "comet/core/windows.h"
#else
#include <signal.h>
#endif  // COMET_MSVC

#include "comet/core/file_system/file_system.h"
#include "comet/core/type/tstring.h"
#include "editor/memory/memory.h"

namespace comet {
namespace editor {
void CometEditor::Update(f64& lag) {
  Engine::Update(lag);
  camera_handler_->Update();
}

void CometEditor::PreLoad() {
  Engine::PreLoad();
  COMET_ATTACH_CUSTOM_MEMORY_LABEL_FUNC(memory::GetEditorMemoryTagLabel);
}

void CometEditor::Load() {
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
  sig_handler.sa_handler = [](s32) { CometEditor::Get().Quit(); };

  sigemptyset(&sig_handler.sa_mask);
  sig_handler.sa_flags = 0;

  sigaction(SIGINT, &sig_handler, NULL);
#endif  // COMET_UNIX

  LoadTmpCode();
  auto& asset_manager{asset::AssetManager::Get()};
  asset_manager.Initialize();
  asset_manager.Refresh();
  Engine::Load();
}

// TODO(m4jr0): Remove temporary code.
void CometEditor::PostLoad() {
  Engine::PostLoad();
  camera_handler_ = std::make_unique<CameraHandler>();
  camera_handler_->Initialize();
}

void CometEditor::PostUnload() {
  camera_handler_->Shutdown();
  asset::AssetManager::Get().Shutdown();
  Engine::PostUnload();
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
  comet::Remove(asset::AssetManager::Get().GetAssetsRootPath() /
                COMET_CTSTRING_VIEW("models/kate/kate.fbx.meta"));
  comet::Remove(
      asset::AssetManager::Get().GetAssetsRootPath() /
      COMET_CTSTRING_VIEW("shaders/vulkan/default_shader.vk.cshader.meta"));
  comet::Remove(asset::AssetManager::Get().GetAssetsRootPath() /
                COMET_CTSTRING_VIEW("shaders/vulkan/default.vk.vert.meta"));
  comet::Remove(asset::AssetManager::Get().GetAssetsRootPath() /
                COMET_CTSTRING_VIEW("shaders/vulkan/default.vk.frag.meta"));
  comet::Remove(asset::AssetManager::Get().GetAssetsRootPath() /
                COMET_CTSTRING_VIEW("shaders/vulkan/default.vk.comp.meta"));
  // comet::Remove(asset::AssetManager::Get().GetAssetsRootPath() /
  //               COMET_CTSTRING_VIEW("models/nanosuit/nanosuit.obj.meta"));
  //  comet::Remove(asset::AssetManager::Get().GetAssetsRootPath() /
  //                COMET_CTSTRING_VIEW("models/sponza/sponza.obj.meta"));
}
}  // namespace editor

std::unique_ptr<Engine> GenerateEngine() {
  return std::make_unique<editor::CometEditor>();
}
}  // namespace comet