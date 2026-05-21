// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet/rendering/comet_rendering_pch.h"
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "comet/render/driver/opengl/handler/opengl_camera_handler.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/math/vector.h"
#include "comet/profiler/profiler.h"
#include "comet/rendering/driver/opengl/opengl_debug.h"
#include "comet/rendering/driver/opengl/type/opengl_camera.h"

namespace comet {
namespace rendering {
namespace gl {
CameraHandler::CameraHandler(const CameraHandlerDescr& descr)
    : Handler{descr} {}

void CameraHandler::Update(const frame::FramePacket* packet) {
  COMET_PROFILE("CameraHandler::Update");
  COMET_ASSERT(packet != nullptr, "CameraHandler::Update",
               "frame packet is null");
  COMET_ASSERT(packet->camera_views != nullptr, "CameraHandler::Update",
               "camera views are null");

  const auto frame_index{frame_state_->GetFrameInFlightIndex()};

  EnsureBuffersInitialized(frame_index);
  UploadCameraDatas(frame_index, packet);
}

CameraGpuData CameraHandler::GetGpuData(
    FrameInFlightIndex frame_index) const noexcept {
  CameraGpuData gpu_data{};

  if (frame_index >= ssbo_camera_datas_.GetSize()) {
    return gpu_data;
  }

  gpu_data.ssbo_camera_datas_handle = ssbo_camera_datas_[frame_index];
  gpu_data.ssbo_camera_datas_size = ssbo_camera_datas_sizes_[frame_index];

  return gpu_data;
}

void CameraHandler::PopulateCameraReadBarrier(
    [[maybe_unused]] FrameInFlightIndex frame_index) const {
  // OpenGL commands are ordered here. Call glMemoryBarrier manually at the call
  // site only if a compute shader writes this buffer before camera reads.
}

void CameraHandler::OnInitialize() {
  const auto frame_count{frame_state_->GetMaxFramesInFlight()};

  ssbo_camera_datas_ =
      Array<GLuint>::WithCapacity(&platform_allocator_, frame_count);
  ssbo_camera_datas_sizes_ =
      Array<usize>::WithCapacity(&platform_allocator_, frame_count);

  ssbo_camera_datas_.Resize(frame_count);
  ssbo_camera_datas_sizes_.Resize(frame_count);

  for (usize i{0}; i < frame_count; ++i) {
    ssbo_camera_datas_[i] = kInvalidGlNativeStorageHandle;
    ssbo_camera_datas_sizes_[i] = 0;
  }
}

void CameraHandler::OnShutdown() {
  for (auto& buffer_handle : ssbo_camera_datas_) {
    DestroyBuffer(buffer_handle);
  }

  ssbo_camera_datas_.Release();
  ssbo_camera_datas_sizes_.Release();
}

void CameraHandler::EnsureBuffersInitialized(FrameInFlightIndex frame_index) {
  COMET_ASSERT(frame_index < ssbo_camera_datas_.GetSize(),
               "CameraHandler::EnsureBuffersInitialized",
               "frame index out of bounds", "frame_index", frame_index,
               "buffer_count", ssbo_camera_datas_.GetSize());

  auto& buffer_handle{ssbo_camera_datas_[frame_index]};

  if (buffer_handle != kInvalidGlNativeStorageHandle) {
    return;
  }

  glCreateBuffers(1, &buffer_handle);
  COMET_ASSERT(buffer_handle != kInvalidGlNativeStorageHandle,
               "CameraHandler::EnsureBuffersInitialized",
               "failed to create camera data buffer");

  COMET_GL_SET_STORAGE_DEBUG_LABEL(buffer_handle, "ssbo_camera_datas_");
}

void CameraHandler::UploadCameraDatas(FrameInFlightIndex frame_index,
                                      const frame::FramePacket* packet) {
  const auto camera_count{packet->camera_views->GetSize()};

  if (camera_count == 0) {
    return;
  }

  const auto required_size{camera_count * sizeof(GpuCameraData)};

  auto& buffer_handle{ssbo_camera_datas_[frame_index]};
  auto& buffer_size{ssbo_camera_datas_sizes_[frame_index]};

  COMET_ASSERT(buffer_handle != kInvalidGlNativeStorageHandle,
               "CameraHandler::UploadCameraDatas",
               "camera data buffer handle is invalid", "frame_index",
               frame_index);

  glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffer_handle);

  if (buffer_size < required_size) {
    glBufferData(GL_SHADER_STORAGE_BUFFER,
                 static_cast<GLsizeiptr>(required_size), nullptr,
                 GL_DYNAMIC_DRAW);
    buffer_size = required_size;
  }

  auto* memory{static_cast<GpuCameraData*>(
      glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_WRITE_ONLY))};

  COMET_ASSERT(memory != nullptr, "CameraHandler::UploadCameraDatas",
               "failed to map camera data buffer", "frame_index", frame_index,
               "required_size", required_size);

  for (usize i{0}; i < camera_count; ++i) {
    const auto& src{(*packet->camera_views)[i].data};
    auto& dst{memory[i]};

    dst.projection = src.projection_matrix;
    dst.view = src.view_matrix;
    dst.view_position = math::Vec4{src.view_position.x, src.view_position.y,
                                   src.view_position.z, 0.0f};
  }

  glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, kInvalidGlNativeStorageHandle);
}

void CameraHandler::DestroyBuffer(GLuint& buffer_handle) {
  if (buffer_handle == kInvalidGlNativeStorageHandle) {
    return;
  }

  glDeleteBuffers(1, &buffer_handle);
  buffer_handle = kInvalidGlNativeStorageHandle;
}
}  // namespace gl
}  // namespace rendering
}  // namespace comet