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
  COMET_ASSERT(main_camera_, "CameraManager::PopulateMainRenderData",
               "main camera handle is invalid");
  PopulateRenderData(main_camera_, data);
}

void CameraManager::OnInitialize() {
  allocator_.Initialize();
  cameras_ = Array<Camera>{&allocator_};

  RegisterEvents();

  main_camera_ = GenerateMainCamera();
}

void CameraManager::OnShutdown() {
  UnregisterEvents();

  main_camera_.Invalidate();
  cameras_.Release();
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

void CameraManager::RegisterEvents() {
  auto& event_manager{event::EventManager::Get()};
  const auto event_function{
      [this](const event::Event& event) { OnEvent(event); }};

  window_initialized_listener_id_ = event_manager.Register(
      event_function, WindowInitializedEvent::kStaticType_);
  COMET_ASSERT(
      window_initialized_listener_id_ != event::kInvalidEventListenerId,
      "CameraManager::RegisterEvents",
      "window initialized listener registration failed");

  window_resize_listener_id_ =
      event_manager.Register(event_function, WindowResizeEvent::kStaticType_);
  COMET_ASSERT(window_resize_listener_id_ != event::kInvalidEventListenerId,
               "CameraManager::RegisterEvents",
               "window resize listener registration failed");
}

void CameraManager::UnregisterEvents() {
  auto& event_manager{event::EventManager::Get()};

  if (window_initialized_listener_id_ != event::kInvalidEventListenerId) {
    event_manager.Unregister(window_initialized_listener_id_);
    window_initialized_listener_id_ = event::kInvalidEventListenerId;
  }

  if (window_resize_listener_id_ != event::kInvalidEventListenerId) {
    event_manager.Unregister(window_resize_listener_id_);
    window_resize_listener_id_ = event::kInvalidEventListenerId;
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
  COMET_ASSERT(camera_pool_.IsAlive(handle), "CameraManager::GetCamera",
               "camera is not alive", "handle", handle);

  const auto index{static_cast<usize>(handle.GetIndex())};
  COMET_ASSERT(index < cameras_.GetSize(), "CameraManager::GetCamera",
               "camera index out of bounds", "index", index, "camera_count",
               cameras_.GetSize());

  auto& camera{cameras_[index]};
  COMET_ASSERT(camera.handle == handle, "CameraManager::GetCamera",
               "camera handle mismatch", "requested_handle", handle,
               "stored_handle", camera.handle);

  return &camera;
}

const Camera* CameraManager::GetCamera(CameraHandle handle) const {
  COMET_ASSERT(camera_pool_.IsAlive(handle), "CameraManager::GetCamera",
               "camera is not alive", "handle", handle);

  const auto index{static_cast<usize>(handle.GetIndex())};
  COMET_ASSERT(index < cameras_.GetSize(), "CameraManager::GetCamera",
               "camera index out of bounds", "index", index, "camera_count",
               cameras_.GetSize());

  const auto& camera{cameras_[index]};
  COMET_ASSERT(camera.handle == handle, "CameraManager::GetCamera",
               "camera handle mismatch", "requested_handle", handle,
               "stored_handle", camera.handle);

  return &camera;
}
}  // namespace rendering
}  // namespace comet