// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "camera_manager.h"

#include "comet/event/window_event.h"

namespace comet {
namespace rendering {
CameraManager::CameraManager(const CameraManagerDescr& descr)
    : Manager{descr}, event_manager_{descr.event_manager} {
  COMET_ASSERT(event_manager_ != nullptr, "Event manager is null!");
}

void CameraManager::Initialize() {
  Manager::Initialize();

  event_manager_->Register(COMET_EVENT_BIND_FUNCTION(CameraManager::OnEvent),
                           event::WindowInitializedEvent::kStaticType_);
  event_manager_->Register(COMET_EVENT_BIND_FUNCTION(CameraManager::OnEvent),
                           event::WindowResizeEvent::kStaticType_);

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
  if (event.GetType() == event::WindowInitializedEvent::kStaticType_) {
    const auto& window_initialized_event{
        static_cast<const event::WindowInitializedEvent&>(event)};

    main_camera_->SetSize(window_initialized_event.GetWidth(),
                          window_initialized_event.GetHeight());
  } else if (event.GetType() == event::WindowResizeEvent::kStaticType_) {
    const auto& window_resize_event{
        static_cast<const event::WindowResizeEvent&>(event)};

    main_camera_->SetSize(window_resize_event.GetWidth(),
                          window_resize_event.GetHeight());
  }
}

void CameraManager::GenerateMainCamera() {
  main_camera_ = std::make_unique<Camera>();
}
}  // namespace rendering
}  // namespace comet
