// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet/rendering/comet_rendering_pch.h"
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "camera_manager.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/event/event_manager.h"
#include "comet/rendering/window/window_event.h"

namespace comet {
namespace rendering {
CameraManager& CameraManager::Get() {
  static CameraManager singleton{};
  return singleton;
}

CameraHandle CameraManager::GetMainCamera() const noexcept {
  return main_camera_;
}

bool CameraManager::IsAlive(CameraHandle handle) const noexcept {
  return camera_pool_.IsAlive(handle);
}

void CameraManager::Reset(CameraHandle handle) { GetCamera(handle)->Reset(); }

void CameraManager::Translate(CameraHandle handle,
                              const math::Vec3& translation) {
  GetCamera(handle)->Translate(translation);
}

void CameraManager::Move(CameraHandle handle, const math::Vec3& delta) {
  GetCamera(handle)->Move(delta);
}

void CameraManager::Rotate(CameraHandle handle, const math::Vec2& delta) {
  GetCamera(handle)->Rotate(delta);
}

void CameraManager::Rotate(CameraHandle handle, const math::Quat& rotation) {
  GetCamera(handle)->Rotate(rotation);
}

void CameraManager::Orbit(CameraHandle handle, const math::Vec2& delta) {
  GetCamera(handle)->Orbit(delta);
}

void CameraManager::SetPosition(CameraHandle handle,
                                const math::Vec3& position) {
  GetCamera(handle)->SetPosition(position);
}

void CameraManager::SetRotation(CameraHandle handle,
                                const math::Quat& rotation) {
  GetCamera(handle)->SetRotation(rotation);
}

void CameraManager::SetWidth(CameraHandle handle, WindowSize width) {
  GetCamera(handle)->SetWidth(width);
}

void CameraManager::SetHeight(CameraHandle handle, WindowSize height) {
  GetCamera(handle)->SetHeight(height);
}

void CameraManager::SetSize(CameraHandle handle, WindowSize width,
                            WindowSize height) {
  GetCamera(handle)->SetSize(width, height);
}

WindowSize CameraManager::GetWidth(CameraHandle handle) const {
  return GetCamera(handle)->GetWidth();
}

WindowSize CameraManager::GetHeight(CameraHandle handle) const {
  return GetCamera(handle)->GetHeight();
}

void CameraManager::PopulateRenderData(CameraHandle handle,
                                       RenderCameraData& data) {
  GetCamera(handle)->PopulateData(data);
}

void CameraManager::PopulateMainRenderData(RenderCameraData& data) {
  PopulateRenderData(main_camera_, data);
}

void CameraManager::OnInitialize() {
  allocator_.Initialize();
  cameras_ = Array<Camera>{&allocator_};

  auto& event_manager{event::EventManager::Get()};
  const auto event_function{
      [this](const event::Event& event) { OnEvent(event); }};
  event_manager.Register(event_function, WindowInitializedEvent::kStaticType_);
  event_manager.Register(event_function, WindowResizeEvent::kStaticType_);

  main_camera_ = GenerateMainCamera();
}

void CameraManager::OnShutdown() {
  main_camera_.Invalidate();
  cameras_.Destroy();
  camera_pool_.Destroy();
  allocator_.Destroy();
}

void CameraManager::OnEvent(const event::Event& event) {
  if (!main_camera_ || !IsAlive(main_camera_)) {
    return;
  }

  if (event.GetType() == WindowInitializedEvent::kStaticType_) {
    const auto& e{static_cast<const WindowInitializedEvent&>(event)};
    SetSize(main_camera_, e.GetWidth(), e.GetHeight());
    return;
  }

  if (event.GetType() == WindowResizeEvent::kStaticType_) {
    const auto& e{static_cast<const WindowResizeEvent&>(event)};
    SetSize(main_camera_, e.GetWidth(), e.GetHeight());
  }
}

CameraHandle CameraManager::GenerateMainCamera() {
  const auto handle{camera_pool_.Generate()};
  const auto index{static_cast<usize>(handle.GetIndex())};

  if (index >= cameras_.GetSize()) {
    cameras_.Resize(index + 1);
  }

  cameras_[index] = Camera{};
  cameras_[index].handle = handle;
  return handle;
}

Camera* CameraManager::GetCamera(CameraHandle handle) {
  COMET_ASSERT(camera_pool_.IsAlive(handle), "Camera ", handle,
               " is not alive!");

  const auto index{static_cast<usize>(handle.GetIndex())};
  COMET_ASSERT(index < cameras_.GetSize(),
               "Camera handle index is out of bounds!");

  auto& camera{cameras_[index]};
  COMET_ASSERT(camera.handle == handle, "Camera handle mismatch!");
  return &camera;
}

const Camera* CameraManager::GetCamera(CameraHandle handle) const {
  COMET_ASSERT(camera_pool_.IsAlive(handle), "Camera ", handle,
               " is not alive!");

  const auto index{static_cast<usize>(handle.GetIndex())};
  COMET_ASSERT(index < cameras_.GetSize(),
               "Camera handle index is out of bounds!");

  const auto& camera{cameras_[index]};
  COMET_ASSERT(camera.handle == handle, "Camera handle mismatch!");
  return &camera;
}
}  // namespace rendering
}  // namespace comet