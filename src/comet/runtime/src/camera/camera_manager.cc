// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_runtime_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "comet/runtime/camera/camera_manager.h"
////////////////////////////////////////////////////////////////////////////////

#ifdef COMET_DEBUG
#include "comet/core/string/c_string.h"
#include "comet/render/debug/rendering_debug_settings.h"
#endif  // COMET_DEBUG

#include "comet/runtime/event/event_manager.h"
#include "comet/core/math/geometry.h"
#include "comet/core/math/math_scalar.h"
#include "comet/core/math/numeric_utils.h"
#include "comet/core/math/quaternion.h"
#include "comet/runtime/camera/camera_utils.h"
#include "comet/platform/window/window_event.h"

namespace comet {
namespace camera {
CameraManager& CameraManager::Get() {
  static CameraManager singleton{};
  return singleton;
}

CameraHandle CameraManager::GetMainCamera() const noexcept {
  return main_camera_;
}

#ifdef COMET_DEBUG
CameraHandle CameraManager::GetDebugCamera() const noexcept {
  return debug_camera_;
}
#endif  // COMET_DEBUG

bool CameraManager::IsAlive(CameraHandle handle) const noexcept {
  return camera_pool_.IsAlive(handle);
}

CameraHandle CameraManager::Generate(const CameraDescr& descr) {
  const auto handle{camera_pool_.Generate()};
  const auto index{static_cast<usize>(handle.GetIndex())};

  if (index >= cameras_.GetSize()) {
    cameras_.Resize(index + 1);
  }

  auto& camera{cameras_[index]};
  camera = {};
  camera.handle = handle;
  camera.position = descr.pose.position;
  camera.rotation = math::GetNormalizedCopy(descr.pose.rotation);
  camera.reset_pose = descr.reset_pose;
  camera.reset_pose.rotation =
      math::GetNormalizedCopy(camera.reset_pose.rotation);
  camera.fov = descr.fov;
  camera.z_near = descr.z_near;
  camera.z_far = descr.z_far;

  if (camera.fov < 1.0f) {
    camera.fov = 1.0f;
  } else if (camera.fov > 179.0f) {
    camera.fov = 179.0f;
  }

  if (camera.z_near < 0.001f) {
    camera.z_near = 0.001f;
  }

  if (camera.z_far <= camera.z_near) {
    camera.z_far = camera.z_near + 0.001f;
  }

  RefreshBasis(camera);

#ifdef COMET_DEBUG
  SetDebugLabel(handle, descr.debug_label);
#endif  // COMET_DEBUG

  return handle;
}

void CameraManager::Destroy(CameraHandle handle) {
  if (!handle) {
    return;
  }

  if (handle == main_camera_) {
    COMET_ASSERT(false, "CameraManager::Destroy", "cannot destroy main camera");
    return;
  }

#ifdef COMET_DEBUG
  if (handle == debug_camera_) {
    COMET_ASSERT(false, "CameraManager::Destroy",
                 "cannot destroy debug camera");
    return;
  }
#endif  // COMET_DEBUG

  COMET_ASSERT(IsAlive(handle), "CameraManager::Destroy", "camera is not alive",
               "handle", handle);

  if (!IsAlive(handle)) {
    return;
  }

  const auto index{static_cast<usize>(handle.GetIndex())};
  cameras_[index] = {};
  camera_pool_.Destroy(handle);
}

void CameraManager::SetProjectionFlags(CameraProjectionFlags flags) noexcept {
  projection_flags_ = flags;
}

CameraProjectionFlags CameraManager::GetProjectionFlags() const noexcept {
  return projection_flags_;
}

usize CameraManager::PopulateCameraViews(Array<CameraView>& views,
                                         WindowSize full_width,
                                         WindowSize full_height) {
  views.Clear();

#ifdef COMET_DEBUG
  const auto& camera_settings{debug::RenderingDebugSettings::Get()};
  const auto use_debug_camera{camera_settings.IsDebugCameraEnabled()};
  const auto draw_debug_on_game{camera_settings.GetDebugDrawFlags(false) !=
                                debug::kDebugDrawFlagBitsNone};
  const auto draw_debug_on_debug{camera_settings.GetDebugDrawFlags(true) !=
                                 debug::kDebugDrawFlagBitsNone};
#else
  constexpr bool use_debug_camera{false};
  constexpr bool draw_debug_on_game{false};
  [[maybe_unused]] constexpr bool draw_debug_on_debug{false};
#endif  // COMET_DEBUG

  const u32 camera_count{use_debug_camera ? 2u : 1u};
  views.Reserve(camera_count);

  const auto full_width_f{static_cast<f32>(full_width)};
  const auto full_height_f{static_cast<f32>(full_height)};
  const auto camera_width{full_width_f / static_cast<f32>(camera_count)};
  const auto aspect_ratio{full_height_f > .0f ? camera_width / full_height_f
                                              : 1.0f};

  const auto add_camera_view = [&](CameraHandle camera, CameraKind kind,
                                   CameraFlags flags,
                                   bool is_debug_draw_enabled, u32 index) {
    auto& view{views.EmplaceLast()};
    view.camera = camera;
    view.kind = kind;
    view.flags = flags;
    view.is_debug_draw_enabled = is_debug_draw_enabled;

    view.viewport = {
        .x = camera_width * static_cast<f32>(index),
        .y = .0f,
        .width = math::Max(camera_width, 1.0f),
        .height = math::Max(full_height_f, 1.0f),
    };

    PopulateRenderData(camera, view.data, aspect_ratio);
  };

  add_camera_view(main_camera_, CameraKind::Game,
                  kCameraFlagBitsGame | kCameraFlagBitsMain, draw_debug_on_game,
                  0);

#ifdef COMET_DEBUG
  if (use_debug_camera) {
    add_camera_view(debug_camera_, CameraKind::Debug, kCameraFlagBitsDebug,
                    draw_debug_on_debug, 1);
  }
#endif  // COMET_DEBUG

  return 0;
}

void CameraManager::Reset(CameraHandle handle) {
  auto& camera{*GetCamera(handle)};
  const auto width{camera.width};
  const auto height{camera.height};

  camera.position = camera.reset_pose.position;
  camera.rotation = math::GetNormalizedCopy(camera.reset_pose.rotation);
  camera.width = width;
  camera.height = height;

  RefreshBasis(camera);
}

void CameraManager::Translate(CameraHandle handle,
                              const math::Vec3& translation) {
  GetCamera(handle)->position += translation;
}

void CameraManager::Move(CameraHandle handle, const math::Vec3& delta) {
  auto& camera{*GetCamera(handle)};
  camera.position +=
      camera.front * delta.z + camera.right * delta.x + camera.up * delta.y;
}

void CameraManager::Rotate(CameraHandle handle, const math::Vec2& delta) {
  auto& camera{*GetCamera(handle)};
  Rotate(handle, GetRotationDelta(camera, delta));
}

void CameraManager::Rotate(CameraHandle handle, const math::Quat& rotation) {
  auto& camera{*GetCamera(handle)};
  camera.rotation = rotation * camera.rotation;
  RefreshBasis(camera);
}

void CameraManager::Orbit(CameraHandle handle, const math::Vec2& delta) {
  auto& camera{*GetCamera(handle)};
  const auto pivot{GetCenterPivotPoint(camera)};
  const auto rotation{GetRotationDelta(camera, delta)};

  camera.rotation = rotation * camera.rotation;
  camera.position = pivot + (rotation * (camera.position - pivot));
  RefreshBasis(camera);
}

void CameraManager::SetPosition(CameraHandle handle,
                                const math::Vec3& position) {
  GetCamera(handle)->position = position;
}

void CameraManager::SetRotation(CameraHandle handle,
                                const math::Quat& rotation) {
  auto& camera{*GetCamera(handle)};
  camera.rotation = math::GetNormalizedCopy(rotation);
  RefreshBasis(camera);
}

void CameraManager::SetSize(CameraHandle handle, WindowSize width,
                            WindowSize height) {
  auto& camera{*GetCamera(handle)};
  camera.width = width;
  camera.height = height;
}

void CameraManager::SetSize(CameraHandle handle, WindowExtent extent) {
  SetSize(handle, extent.width, extent.height);
}

void CameraManager::SetWidth(CameraHandle handle, WindowSize width) {
  GetCamera(handle)->width = width;
}

void CameraManager::SetHeight(CameraHandle handle, WindowSize height) {
  GetCamera(handle)->height = height;
}

void CameraManager::SetFov(CameraHandle handle, f32 fov) {
  auto& camera{*GetCamera(handle)};

  if (fov < 1.0f) {
    fov = 1.0f;
  } else if (fov > 179.0f) {
    fov = 179.0f;
  }

  camera.fov = fov;
}

void CameraManager::SetNearPlane(CameraHandle handle, f32 value) {
  auto& camera{*GetCamera(handle)};

  if (value < 0.001f) {
    value = 0.001f;
  }

  camera.z_near = value;

  if (camera.z_far <= camera.z_near) {
    camera.z_far = camera.z_near + 0.001f;
  }
}

void CameraManager::SetFarPlane(CameraHandle handle, f32 value) {
  auto& camera{*GetCamera(handle)};

  if (value <= camera.z_near) {
    value = camera.z_near + 0.001f;
  }

  camera.z_far = value;
}

void CameraManager::SetResetPose(CameraHandle handle, const CameraPose& pose) {
  auto& camera{*GetCamera(handle)};
  camera.reset_pose.position = pose.position;
  camera.reset_pose.rotation = math::GetNormalizedCopy(pose.rotation);
}

const math::Vec3& CameraManager::GetPosition(CameraHandle handle) const {
  return GetCamera(handle)->position;
}

const math::Quat& CameraManager::GetRotation(CameraHandle handle) const {
  return GetCamera(handle)->rotation;
}

const math::Vec3& CameraManager::GetUp(CameraHandle handle) const {
  return GetCamera(handle)->up;
}

const math::Vec3& CameraManager::GetFront(CameraHandle handle) const {
  return GetCamera(handle)->front;
}

const math::Vec3& CameraManager::GetRight(CameraHandle handle) const {
  return GetCamera(handle)->right;
}

WindowExtent CameraManager::GetSize(CameraHandle handle) const {
  const auto& camera{*GetCamera(handle)};
  return {.width = camera.width, .height = camera.height};
}

WindowSize CameraManager::GetWidth(CameraHandle handle) const {
  return GetCamera(handle)->width;
}

WindowSize CameraManager::GetHeight(CameraHandle handle) const {
  return GetCamera(handle)->height;
}

f32 CameraManager::GetFov(CameraHandle handle) const {
  return GetCamera(handle)->fov;
}

f32 CameraManager::GetFovInRadians(CameraHandle handle) const {
  return math::ConvertToRadians(GetCamera(handle)->fov);
}

f32 CameraManager::GetNearPlane(CameraHandle handle) const {
  return GetCamera(handle)->z_near;
}

CameraPose CameraManager::GetResetPose(CameraHandle handle) const {
  return GetCamera(handle)->reset_pose;
}
f32 CameraManager::GetFarPlane(CameraHandle handle) const {
  return GetCamera(handle)->z_far;
}

f32 CameraManager::GetAspectRatio(CameraHandle handle) const {
  const auto& camera{*GetCamera(handle)};

  if (camera.height == 0) {
    return 1.0f;
  }

  return static_cast<f32>(camera.width) / static_cast<f32>(camera.height);
}

#ifdef COMET_DEBUG
void CameraManager::SetDebugLabel(CameraHandle handle, const schar* label) {
  auto& camera{*GetCamera(handle)};

  if (label == nullptr || comet::IsEmpty(label)) {
    comet::Clear(camera.debug_label, kCameraDebugLabelCapacity);
    return;
  }

  const auto length{
      math::Min(comet::GetLength(label),
                static_cast<usize>(kCameraDebugLabelCapacity - 1))};

  comet::Copy(camera.debug_label, label, length);
  camera.debug_label[length] = '\0';
}

const schar* CameraManager::GetDebugLabel(CameraHandle handle) const {
  const auto& camera{*GetCamera(handle)};

  if (comet::IsEmpty(camera.debug_label)) {
    return nullptr;
  }

  return camera.debug_label;
}
#endif  // COMET_DEBUG

void CameraManager::OnInitialize() {
  allocator_.Initialize();
  cameras_ = Array<CameraEntry>{&allocator_};

  RegisterEvents();

  main_camera_ = GenerateMainCamera();

#ifdef COMET_DEBUG
  debug_camera_ = GenerateDebugCamera();
#endif  // COMET_DEBUG
}

void CameraManager::OnShutdown() {
  UnregisterEvents();

#ifdef COMET_DEBUG
  debug_camera_.Invalidate();
#endif  // COMET_DEBUG

  main_camera_.Invalidate();
  cameras_.Release();
  camera_pool_.Destroy();
  allocator_.Destroy();
}

void CameraManager::OnEvent(const event::Event& event) {
  if (!main_camera_ || !IsAlive(main_camera_)) {
    return;
  }

  if (event.GetType() == platform::WindowInitializedEvent::kStaticType_) {
    const auto& e{static_cast<const platform::WindowInitializedEvent&>(event)};
    SetSize(main_camera_, e.GetWidth(), e.GetHeight());

#ifdef COMET_DEBUG
    if (debug_camera_ && IsAlive(debug_camera_)) {
      SetSize(debug_camera_, e.GetWidth(), e.GetHeight());
    }
#endif  // COMET_DEBUG

    return;
  }

  if (event.GetType() == platform::WindowResizeEvent::kStaticType_) {
    const auto& e{static_cast<const platform::WindowResizeEvent&>(event)};
    SetSize(main_camera_, e.GetWidth(), e.GetHeight());

#ifdef COMET_DEBUG
    if (debug_camera_ && IsAlive(debug_camera_)) {
      SetSize(debug_camera_, e.GetWidth(), e.GetHeight());
    }
#endif  // COMET_DEBUG
  }
}

void CameraManager::RegisterEvents() {
  auto& event_manager{event::EventManager::Get()};
  const auto event_function{
      [this](const event::Event& event) { OnEvent(event); }};

  window_initialized_listener_id_ = event_manager.Register(
      event_function, platform::WindowInitializedEvent::kStaticType_);
  COMET_ASSERT(
      window_initialized_listener_id_ != event::kInvalidEventListenerId,
      "CameraManager::RegisterEvents",
      "window initialized listener registration failed");

  window_resize_listener_id_ = event_manager.Register(
      event_function, platform::WindowResizeEvent::kStaticType_);
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
  CameraDescr descr{};
#ifdef COMET_DEBUG
  descr.debug_label = "Game Camera";
#endif
  return Generate(descr);
}

#ifdef COMET_DEBUG
CameraHandle CameraManager::GenerateDebugCamera() {
  CameraDescr descr{};
  descr.z_far = 2000.0f;
  descr.debug_label = "Debug Camera";
  return Generate(descr);
}
#endif

void CameraManager::PopulateRenderData(CameraHandle handle,
                                       CameraViewData& data,
                                       f32 aspect_ratio) const {
  const auto& camera{*GetCamera(handle)};

  data.projection_matrix = GenerateProjectionMatrix(camera, aspect_ratio);
  data.view_matrix = GenerateViewMatrix(camera);
  data.view_position = camera.position;
  data.front = camera.front;
  data.up = camera.up;
  data.right = camera.right;
  data.near_plane = camera.z_near;
  data.far_plane = camera.z_far;
  data.fov_y_radians = math::ConvertToRadians(camera.fov);
  data.aspect_ratio = aspect_ratio;
}

math::Mat4 CameraManager::GenerateProjectionMatrix(const CameraEntry& camera,
                                                   f32 aspect_ratio) const {
  const auto depth_range{
      (projection_flags_ & kProjectionFlagBitsDepthZeroToOne) != 0
          ? ClipSpaceDepthRange::ZeroToOne
          : ClipSpaceDepthRange::MinusOneToOne};

  auto projection{GeneratePerspectiveMatrix(math::ConvertToRadians(camera.fov),
                                            aspect_ratio, camera.z_near,
                                            camera.z_far, depth_range)};

  if ((projection_flags_ & kProjectionFlagBitsInvertY) != 0) {
    projection[1][1] *= -1.0f;
  }

  return projection;
}

math::Mat4 CameraManager::GenerateViewMatrix(const CameraEntry& camera) const {
  return LookAt(camera.position, camera.position + camera.front, camera.up);
}

void CameraManager::RefreshBasis(CameraEntry& camera) {
  camera.rotation = math::GetNormalizedCopy(camera.rotation);
  camera.front = camera.rotation * -kWorldFront_;
  camera.right = camera.rotation * kWorldRight_;

  math::Normalize(camera.front);
  math::Normalize(camera.right);

  camera.up = math::Cross(camera.right, camera.front);
  math::Normalize(camera.up);
}

math::Vec3 CameraManager::GetCenterPivotPoint(const CameraEntry& camera) const {
  const auto& ray_direction{camera.front};
  const auto& ray_origin{camera.position};

  if (math::IsAlmostZero(ray_direction.z)) {
    return ray_origin + ray_direction;
  }

  const auto t{-ray_origin.z / ray_direction.z};
  return ray_origin + ray_direction * t;
}

math::Quat CameraManager::GetRotationDelta(const CameraEntry& camera,
                                           const math::Vec2& delta) const {
  const auto yaw_rotation{math::GetQuaternionRotation(delta.x, kWorldUp_)};
  const auto pitch_rotation{math::GetQuaternionRotation(delta.y, camera.right)};
  return yaw_rotation * pitch_rotation;
}

CameraManager::CameraEntry* CameraManager::GetCamera(CameraHandle handle) {
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

const CameraManager::CameraEntry* CameraManager::GetCamera(
    CameraHandle handle) const {
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
}  // namespace camera
}  // namespace comet