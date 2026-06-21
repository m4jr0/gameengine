// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_RENDER_DRIVER_OPENGL_HANDLER_OPENGL_CAMERA_HANDLER_H_
#define COMET_RENDER_DRIVER_OPENGL_HANDLER_OPENGL_CAMERA_HANDLER_H_

// External. ///////////////////////////////////////////////////////////////////
#include "glad/glad.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/runtime/frame/frame_packet.h"
#include "comet/runtime/memory/allocator/platform_allocator.h"
#include "comet/core/container/array.h"
#include "comet/render/driver/opengl/handler/opengl_handler.h"
#include "comet/render/driver/opengl/type/opengl_frame.h"
#include "comet/render/driver/opengl/type/opengl_storage.h"

namespace comet {
namespace render {
namespace gl {
struct CameraGpuData {
  GLuint ssbo_camera_datas_handle{kInvalidGlNativeStorageHandle};
  usize ssbo_camera_datas_size{0};
};

struct CameraHandlerDescr : HandlerDescr {};

class CameraHandler : public Handler {
 public:
  explicit CameraHandler(const CameraHandlerDescr& descr);
  CameraHandler(const CameraHandler&) = delete;
  CameraHandler(CameraHandler&&) = delete;
  CameraHandler& operator=(const CameraHandler&) = delete;
  CameraHandler& operator=(CameraHandler&&) = delete;
  ~CameraHandler() override = default;

  void Update(const frame::FramePacket* packet);

  CameraGpuData GetGpuData(FrameInFlightIndex frame_index) const noexcept;

  void PopulateCameraReadBarrier(FrameInFlightIndex frame_index) const;

 protected:
  void OnInitialize() override;
  void OnShutdown() override;

 private:
  void EnsureBuffersInitialized(FrameInFlightIndex frame_index);
  void UploadCameraDatas(FrameInFlightIndex frame_index,
                         const frame::FramePacket* packet);
  void DestroyBuffer(GLuint& buffer_handle);

  memory::PlatformAllocator platform_allocator_{
      kEngineMemoryTagRender};

  Array<GLuint> ssbo_camera_datas_{};
  Array<usize> ssbo_camera_datas_sizes_{};
};

}  // namespace gl
}  // namespace render
}  // namespace comet

#endif  // COMET_RENDER_DRIVER_OPENGL_HANDLER_OPENGL_CAMERA_HANDLER_H_