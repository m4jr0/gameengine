// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "comet/rendering/comet_rendering_pch.h"
#include "comet_pch.h"

#include "camera_manager.h"

#include "comet/event/event_manager.h"
#include "comet/rendering/window/window_event.h"

namespace comet {
namespace rendering {
CameraManager& CameraManager::Get() {
  static CameraManager singleton{};
  return singleton;
}

void CameraManager::Initialize() {
  Manager::Initialize();
  auto& event_manager{event::EventManager::Get()};

  event_manager.Register(COMET_EVENT_BIND_FUNCTION(CameraManager::OnEvent),
                         WindowInitializedEvent::kStaticType_);
  event_manager.Register(COMET_EVENT_BIND_FUNCTION(CameraManager::OnEvent),
                         WindowResizeEvent::kStaticType_);

  GenerateMainCamera();
}

void CameraManager::Shutdown() {
  main_camera_ = nullptr;
  Manager::Shutdown();
}

Camera* CameraManager::GetMainCamera() {
  COMET_ASSERT(main_camera_ != nullptr, "Main camera is null!");
  return main_camera_.get();
}

void CameraManager::OnEvent(const event::Event& event) {
  if (event.GetType() == WindowInitializedEvent::kStaticType_) {
    const auto& window_initialized_event{
        static_cast<const WindowInitializedEvent&>(event)};

    main_camera_->SetSize(window_initialized_event.GetWidth(),
                          window_initialized_event.GetHeight());
  } else if (event.GetType() == WindowResizeEvent::kStaticType_) {
    const auto& window_resize_event{
        static_cast<const WindowResizeEvent&>(event)};

    main_camera_->SetSize(window_resize_event.GetWidth(),
                          window_resize_event.GetHeight());
  }
}

void CameraManager::GenerateMainCamera() {
  main_camera_ = std::make_unique<Camera>();
}
}  // namespace rendering
}  // namespace comet
