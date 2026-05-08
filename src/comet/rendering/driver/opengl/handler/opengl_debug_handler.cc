// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet/rendering/comet_rendering_pch.h"
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "opengl_debug_handler.h"
////////////////////////////////////////////////////////////////////////////////

#ifdef COMET_DEBUG_RENDERING

#include "comet/rendering/driver/opengl/opengl_debug.h"
#include "comet/rendering/driver/opengl/type/opengl_mesh.h"

namespace comet {
namespace rendering {
namespace gl {
DebugHandler::DebugHandler(const DebugHandlerDescr& descr)
    : Handler{descr},
      render_proxy_record_store_{descr.render_proxy_record_store} {
  COMET_ASSERT(render_proxy_record_store_ != nullptr,
               "DebugHandler::DebugHandler",
               "render proxy record store is null");
}

void DebugHandler::Update() {
  aabb_count_ =
      static_cast<u32>(render_proxy_record_store_->GetRecordSlotCount());

  EnsureAabbCapacity(GetDebugAabbCount());

  EnsureLineCapacity(GetDebugAabbCount(), GetDebugCameraFrustumCount(),
                     GetDebugCascadeFrustumCount(),
                     GetDebugLightFrustumCount());
}

DebugGpuData DebugHandler::GetGpuData(
    FrameInFlightIndex frame_index) const noexcept {
  DebugGpuData gpu_data{};

  gpu_data.ssbo_debug_data_handle = ssbo_debug_data_handle_[frame_index];
  gpu_data.ssbo_debug_data_size = ssbo_debug_data_size_[frame_index];

  gpu_data.ssbo_debug_lines_handle = ssbo_debug_lines_handle_;
  gpu_data.ssbo_debug_lines_size = ssbo_debug_lines_size_;

  gpu_data.ssbo_debug_aabbs_handle = ssbo_debug_aabbs_handle_;
  gpu_data.ssbo_debug_aabbs_size = ssbo_debug_aabbs_size_;

  gpu_data.ssbo_debug_camera_frustums_handle =
      ssbo_debug_camera_frustums_handle_;
  gpu_data.ssbo_debug_camera_frustums_size = ssbo_debug_camera_frustums_size_;

  gpu_data.ssbo_debug_cascade_frustums_handle =
      ssbo_debug_cascade_frustums_handle_;
  gpu_data.ssbo_debug_cascade_frustums_size = ssbo_debug_cascade_frustums_size_;

  gpu_data.ssbo_debug_light_frustums_handle = ssbo_debug_light_frustums_handle_;
  gpu_data.ssbo_debug_light_frustums_size = ssbo_debug_light_frustums_size_;

  return gpu_data;
}

u32 DebugHandler::GetAabbCount() const noexcept { return aabb_count_; }

void DebugHandler::PrepareCullDebugWrite(FrameInFlightIndex frame_index) {
  auto* debug_data{debug_data_[frame_index]};

  if (debug_data == nullptr) {
    visible_count_ = 0;
    return;
  }

  visible_count_ = debug_data->visible_count;
  debug_data->visible_count = 0;
}

u32 DebugHandler::GetVisibleCount() const noexcept { return visible_count_; }

void DebugHandler::SetDebugCameraFrustum(u32 index,
                                         const math::Mat4& view_proj) {
  if (index >= kMaxDebugCameraFrustumCount_) {
    return;
  }

  if (debug_camera_frustums_.GetSize() <= index) {
    debug_camera_frustums_.Resize(index + 1);
  }

  debug_camera_frustums_[index] = view_proj;

  glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_debug_camera_frustums_handle_);
  glBufferSubData(GL_SHADER_STORAGE_BUFFER,
                  static_cast<GLintptr>(index * sizeof(math::Mat4)),
                  static_cast<GLsizeiptr>(sizeof(math::Mat4)), &view_proj);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, kInvalidGlNativeStorageHandle);
}

void DebugHandler::ClearDebugCameraFrustums() {
  debug_camera_frustums_.Clear();
}

void DebugHandler::SetDebugCascadeFrustumCorners(
    u32 index, const StaticArray<math::Vec3, 8>& corners) {
  if (index >= kMaxDebugCascadeFrustumCount_) {
    return;
  }

  if (debug_cascade_frustums_.GetSize() <= index) {
    debug_cascade_frustums_.Resize(index + 1);
  }

  auto& dst{debug_cascade_frustums_[index]};

  for (u32 i{0}; i < 8; ++i) {
    dst.corners[i] = math::Vec4{corners[i].x, corners[i].y, corners[i].z, 1.0f};
  }

  glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_debug_cascade_frustums_handle_);
  glBufferSubData(GL_SHADER_STORAGE_BUFFER,
                  static_cast<GLintptr>(index * sizeof(DebugFrustumCorners)),
                  static_cast<GLsizeiptr>(sizeof(DebugFrustumCorners)), &dst);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, kInvalidGlNativeStorageHandle);
}

void DebugHandler::ClearDebugCascadeFrustumCorners() {
  debug_cascade_frustums_.Clear();
}

void DebugHandler::SetDebugLightFrustum(u32 index,
                                        const math::Mat4& view_proj) {
  if (index >= kMaxDebugLightFrustumCount_) {
    return;
  }

  if (debug_light_frustums_.GetSize() <= index) {
    debug_light_frustums_.Resize(index + 1);
  }

  debug_light_frustums_[index] = view_proj;

  glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_debug_light_frustums_handle_);
  glBufferSubData(GL_SHADER_STORAGE_BUFFER,
                  static_cast<GLintptr>(index * sizeof(math::Mat4)),
                  static_cast<GLsizeiptr>(sizeof(math::Mat4)), &view_proj);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, kInvalidGlNativeStorageHandle);
}

void DebugHandler::ClearDebugLightFrustums() { debug_light_frustums_.Clear(); }

GlBufferView DebugHandler::GetDebugLineBuffer() const noexcept {
  return {ssbo_debug_lines_handle_, ssbo_debug_lines_size_};
}

GlBufferView DebugHandler::GetDebugAabbBuffer() const noexcept {
  return {ssbo_debug_aabbs_handle_, ssbo_debug_aabbs_size_};
}

GlBufferView DebugHandler::GetDebugCameraFrustumBuffer() const noexcept {
  return {ssbo_debug_camera_frustums_handle_, ssbo_debug_camera_frustums_size_};
}

GlBufferView DebugHandler::GetDebugCascadeFrustumCornersBuffer()
    const noexcept {
  return {ssbo_debug_cascade_frustums_handle_,
          ssbo_debug_cascade_frustums_size_};
}

GlBufferView DebugHandler::GetDebugLightFrustumBuffer() const noexcept {
  return {ssbo_debug_light_frustums_handle_, ssbo_debug_light_frustums_size_};
}

u32 DebugHandler::GetDebugAabbCount() const noexcept { return aabb_count_; }

u32 DebugHandler::GetDebugLineVertexCount() const noexcept {
  return aabb_count_ * kDebugAabbLineVertexCount_ +
         GetDebugCameraFrustumCount() * kDebugFrustumLineVertexCount_ +
         GetDebugCascadeFrustumCount() * kDebugFrustumLineVertexCount_ +
         GetDebugLightFrustumCount() * kDebugFrustumLineVertexCount_;
}

u32 DebugHandler::GetDebugCameraFrustumCount() const noexcept {
  return static_cast<u32>(debug_camera_frustums_.GetSize());
}

u32 DebugHandler::GetDebugCascadeFrustumCount() const noexcept {
  return static_cast<u32>(debug_cascade_frustums_.GetSize());
}

u32 DebugHandler::GetDebugLightFrustumCount() const noexcept {
  return static_cast<u32>(debug_light_frustums_.GetSize());
}

void DebugHandler::OnInitialize() {
  InitializeReadbackData();
  InitializeGpuData();
}

void DebugHandler::OnShutdown() {
  DestroyReadbackData();
  DestroyGpuData();

  aabb_count_ = 0;
  visible_count_ = 0;
}

void DebugHandler::InitializeReadbackData() {
  for (u32 i{0}; i < kDebugDataBufferCount_; ++i) {
    glGenBuffers(1, &ssbo_debug_data_handle_[i]);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_debug_data_handle_[i]);

    COMET_GL_SET_STORAGE_DEBUG_LABEL(ssbo_debug_data_handle_[i],
                                     "ssbo_debug_data_");

    const auto size{static_cast<GLsizei>(sizeof(GpuDebugData))};
    glBufferData(GL_SHADER_STORAGE_BUFFER, size, nullptr, GL_DYNAMIC_READ);
    ssbo_debug_data_size_[i] = size;

    void* debug_data{glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, size,
                                      GL_MAP_READ_BIT | GL_MAP_WRITE_BIT)};

    COMET_ASSERT(debug_data != nullptr, "DebugHandler::InitializeReadbackData",
                 "failed to map debug data buffer", "buffer_index", i);

    debug_data_[i] = static_cast<GpuDebugData*>(debug_data);
  }

  glBindBuffer(GL_SHADER_STORAGE_BUFFER, kInvalidGlNativeStorageHandle);
}

void DebugHandler::DestroyReadbackData() {
  for (u32 i{0}; i < kDebugDataBufferCount_; ++i) {
    if (ssbo_debug_data_handle_[i] == kInvalidGlNativeStorageHandle) {
      continue;
    }

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_debug_data_handle_[i]);

    if (debug_data_[i] != nullptr) {
      glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
      debug_data_[i] = nullptr;
    }

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, kInvalidGlNativeStorageHandle);
    glDeleteBuffers(1, &ssbo_debug_data_handle_[i]);

    ssbo_debug_data_handle_[i] = kInvalidGlNativeStorageHandle;
    ssbo_debug_data_size_[i] = 0;
  }
}

void DebugHandler::InitializeGpuData() {
  debug_cascade_frustums_ = Array<DebugFrustumCorners>::WithCapacity(
      &platform_allocator_, kMaxDebugCascadeFrustumCount_);

  debug_light_frustums_ = Array<math::Mat4>::WithCapacity(
      &platform_allocator_, kMaxDebugLightFrustumCount_);

  debug_camera_frustums_ = Array<math::Mat4>::WithCapacity(
      &platform_allocator_, kMaxDebugCameraFrustumCount_);

  glGenBuffers(1, &ssbo_debug_aabbs_handle_);
  COMET_GL_SET_STORAGE_DEBUG_LABEL(ssbo_debug_aabbs_handle_,
                                   "ssbo_debug_aabbs_");
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_debug_aabbs_handle_);
  ssbo_debug_aabbs_size_ = GetAabbBufferSize(kDefaultDebugAabbCount_);
  glBufferData(GL_SHADER_STORAGE_BUFFER, ssbo_debug_aabbs_size_, nullptr,
               GL_DYNAMIC_DRAW);

  glGenBuffers(1, &ssbo_debug_cascade_frustums_handle_);
  COMET_GL_SET_STORAGE_DEBUG_LABEL(ssbo_debug_cascade_frustums_handle_,
                                   "ssbo_debug_cascade_frustums_");
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_debug_cascade_frustums_handle_);
  ssbo_debug_cascade_frustums_size_ = static_cast<GLsizei>(
      kMaxDebugCascadeFrustumCount_ * sizeof(DebugFrustumCorners));
  glBufferData(GL_SHADER_STORAGE_BUFFER, ssbo_debug_cascade_frustums_size_,
               nullptr, GL_DYNAMIC_DRAW);

  glGenBuffers(1, &ssbo_debug_light_frustums_handle_);
  COMET_GL_SET_STORAGE_DEBUG_LABEL(ssbo_debug_light_frustums_handle_,
                                   "ssbo_debug_light_frustums_");
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_debug_light_frustums_handle_);
  ssbo_debug_light_frustums_size_ =
      static_cast<GLsizei>(kMaxDebugLightFrustumCount_ * sizeof(math::Mat4));
  glBufferData(GL_SHADER_STORAGE_BUFFER, ssbo_debug_light_frustums_size_,
               nullptr, GL_DYNAMIC_DRAW);

  glGenBuffers(1, &ssbo_debug_camera_frustums_handle_);
  COMET_GL_SET_STORAGE_DEBUG_LABEL(ssbo_debug_camera_frustums_handle_,
                                   "ssbo_debug_camera_frustums_");
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_debug_camera_frustums_handle_);
  ssbo_debug_camera_frustums_size_ =
      static_cast<GLsizei>(kMaxDebugCameraFrustumCount_ * sizeof(math::Mat4));
  glBufferData(GL_SHADER_STORAGE_BUFFER, ssbo_debug_camera_frustums_size_,
               nullptr, GL_DYNAMIC_DRAW);

  glGenBuffers(1, &ssbo_debug_lines_handle_);
  COMET_GL_SET_STORAGE_DEBUG_LABEL(ssbo_debug_lines_handle_,
                                   "ssbo_debug_lines_");
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_debug_lines_handle_);
  ssbo_debug_lines_size_ = GetLineBufferSize(
      kDefaultDebugAabbCount_, kMaxDebugCameraFrustumCount_,
      kMaxDebugCascadeFrustumCount_, kMaxDebugLightFrustumCount_);
  glBufferData(GL_SHADER_STORAGE_BUFFER, ssbo_debug_lines_size_, nullptr,
               GL_DYNAMIC_DRAW);

  glBindBuffer(GL_SHADER_STORAGE_BUFFER, kInvalidGlNativeStorageHandle);
}

void DebugHandler::DestroyGpuData() {
  if (ssbo_debug_aabbs_handle_ != kInvalidGlNativeStorageHandle) {
    glDeleteBuffers(1, &ssbo_debug_aabbs_handle_);
    ssbo_debug_aabbs_handle_ = kInvalidGlNativeStorageHandle;
    ssbo_debug_aabbs_size_ = 0;
  }

  if (ssbo_debug_lines_handle_ != kInvalidGlNativeStorageHandle) {
    glDeleteBuffers(1, &ssbo_debug_lines_handle_);
    ssbo_debug_lines_handle_ = kInvalidGlNativeStorageHandle;
    ssbo_debug_lines_size_ = 0;
  }

  if (ssbo_debug_cascade_frustums_handle_ != kInvalidGlNativeStorageHandle) {
    glDeleteBuffers(1, &ssbo_debug_cascade_frustums_handle_);
    ssbo_debug_cascade_frustums_handle_ = kInvalidGlNativeStorageHandle;
    ssbo_debug_cascade_frustums_size_ = 0;
  }

  if (ssbo_debug_light_frustums_handle_ != kInvalidGlNativeStorageHandle) {
    glDeleteBuffers(1, &ssbo_debug_light_frustums_handle_);
    ssbo_debug_light_frustums_handle_ = kInvalidGlNativeStorageHandle;
    ssbo_debug_light_frustums_size_ = 0;
  }

  if (ssbo_debug_camera_frustums_handle_ != kInvalidGlNativeStorageHandle) {
    glDeleteBuffers(1, &ssbo_debug_camera_frustums_handle_);
    ssbo_debug_camera_frustums_handle_ = kInvalidGlNativeStorageHandle;
    ssbo_debug_camera_frustums_size_ = 0;
  }

  debug_cascade_frustums_.Release();
  debug_light_frustums_.Release();
  debug_camera_frustums_.Release();
}

void DebugHandler::EnsureAabbCapacity(u32 count) {
  const auto size{GetAabbBufferSize(count)};

  if (size == 0) {
    return;
  }

  ReallocateBufferIfNeeded(ssbo_debug_aabbs_handle_, ssbo_debug_aabbs_size_,
                           size, GL_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW);
}

void DebugHandler::EnsureLineCapacity(u32 aabb_count, u32 camera_frustum_count,
                                      u32 cascade_frustum_count,
                                      u32 light_frustum_count) {
  const auto size{GetLineBufferSize(aabb_count, camera_frustum_count,
                                    cascade_frustum_count,
                                    light_frustum_count)};

  if (size == 0) {
    return;
  }

  ReallocateBufferIfNeeded(ssbo_debug_lines_handle_, ssbo_debug_lines_size_,
                           size, GL_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW);
}

GLsizei DebugHandler::GetAabbBufferSize(u32 count) const noexcept {
  return static_cast<GLsizei>(count * sizeof(GpuDebugAabb));
}

GLsizei DebugHandler::GetLineBufferSize(
    u32 aabb_count, u32 camera_frustum_count, u32 cascade_frustum_count,
    u32 light_frustum_count) const noexcept {
  const auto vertex_count{aabb_count * kDebugAabbLineVertexCount_ +
                          camera_frustum_count * kDebugFrustumLineVertexCount_ +
                          cascade_frustum_count *
                              kDebugFrustumLineVertexCount_ +
                          light_frustum_count * kDebugFrustumLineVertexCount_};

  return static_cast<GLsizei>(vertex_count * sizeof(GpuDebugLineVertex));
}

void DebugHandler::ReallocateBufferIfNeeded(GlNativeStorageHandle buffer_handle,
                                            GLsizei& buffer_size,
                                            GLsizei required_size,
                                            GLenum target, GLenum usage) {
  COMET_ASSERT(buffer_handle != kInvalidGlNativeStorageHandle,
               "DebugHandler::ReallocateBufferIfNeeded",
               "buffer handle is invalid");

  if (buffer_size >= required_size) {
    return;
  }

  glBindBuffer(target, buffer_handle);
  glBufferData(target, required_size, nullptr, usage);
  glBindBuffer(target, kInvalidGlNativeStorageHandle);

  buffer_size = required_size;
}
}  // namespace gl
}  // namespace rendering
}  // namespace comet

#endif  // COMET_DEBUG_RENDERING