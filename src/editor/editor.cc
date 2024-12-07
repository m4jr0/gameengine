// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "editor.h"

#ifdef COMET_MSVC
#include <iostream>

#include "comet/core/windows.h"
#else
#include <signal.h>
#endif  // COMET_MSVC

#include "comet/core/file_system/file_system.h"
#include "comet/core/memory/memory_utils.h"
#include "comet/core/type/map.h"
#include "comet/core/type/tstring.h"
#include "comet/entity/entity_manager.h"
#include "comet/entity/factory/entity_factory_manager.h"
#include "comet/physics/component/transform_component.h"
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
  PostLoadTmpCode();
}

void CometEditor::PostUnload() {
  PostUnloadTmpCode();
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
  // comet::Remove(asset::AssetManager::Get().GetAssetsRootPath() /
  //               COMET_CTSTRING_VIEW("models/nanosuit/nanosuit.obj.meta"));
  //  comet::Remove(asset::AssetManager::Get().GetAssetsRootPath() /
  //                COMET_CTSTRING_VIEW("models/sponza/sponza.obj.meta"));
}

void CometEditor::PostLoadTmpCode() {
  auto character_id{
      entity::EntityFactoryManager::Get().GetModel()->GenerateSkeletal(
          COMET_CTSTRING_VIEW("models/kate/kate.fbx"))};

  auto* character_transform{
      entity::EntityManager::Get().GetComponent<physics::TransformComponent>(
          character_id)};

  constexpr auto kCharacterScaleFactor{0.15f};
  character_transform->local[0][0] *= kCharacterScaleFactor;
  character_transform->local[1][1] *= kCharacterScaleFactor;
  character_transform->local[2][2] *= kCharacterScaleFactor;

  auto sponza_id{entity::EntityFactoryManager::Get().GetModel()->GenerateStatic(
      COMET_CTSTRING_VIEW("models/sponza/sponza.obj"))};

  auto* sponza_transform{
      entity::EntityManager::Get().GetComponent<physics::TransformComponent>(
          sponza_id)};

  constexpr auto kSponzaScaleFactor{0.17f};
  sponza_transform->local[0][0] *= kSponzaScaleFactor;
  sponza_transform->local[1][1] *= kSponzaScaleFactor;
  sponza_transform->local[2][2] *= kSponzaScaleFactor;

  camera_handler_ = std::make_unique<CameraHandler>();
  camera_handler_->Initialize();

  memory::PlatformAllocator allocator{memory::kEditorMemoryTagAsset};
  allocator.Initialize();
  Map<s32, s32> map_test{&allocator};
  map_test.Set(42, 1337);
  auto test1{map_test[42]};
  allocator.Destroy();
}

void CometEditor::PostUnloadTmpCode() { camera_handler_->Shutdown(); }
}  // namespace editor

std::unique_ptr<Engine> GenerateEngine() {
  return std::make_unique<editor::CometEditor>();
}
}  // namespace comet