// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_RENDER_DRIVER_VULKAN_HANDLER_VULKAN_CAMERA_HANDLER_H_
#define COMET_RENDER_DRIVER_VULKAN_HANDLER_VULKAN_CAMERA_HANDLER_H_

// External. ///////////////////////////////////////////////////////////////////
#include "vulkan/vulkan.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/runtime/frame/frame_packet.h"
#include "comet/runtime/memory/allocator/platform_allocator.h"
#include "comet/core/container/array.h"
#include "comet/render/driver/vulkan/handler/vulkan_handler.h"
#include "comet/render/driver/vulkan/type/vulkan_buffer.h"
#include "comet/render/driver/vulkan/type/vulkan_frame.h"

namespace comet {
namespace rendering {
namespace vk {
struct CameraGpuData {
  VkBuffer ssbo_camera_datas_handle{VK_NULL_HANDLE};
  VkDeviceSize ssbo_camera_datas_size{0};
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

  void PopulateCameraReadBarrier(FrameInFlightIndex frame_index,
                                 Array<VkBufferMemoryBarrier>& out) const;

 protected:
  void OnInitialize() override;
  void OnShutdown() override;

 private:
  void EnsureBuffersInitialized(FrameInFlightIndex frame_index);
  void UploadCameraDatas(FrameInFlightIndex frame_index,
                         const frame::FramePacket* packet);
  void AddUploadReleaseBarrier(FrameInFlightIndex frame_index,
                               const Buffer& buffer);

  memory::PlatformAllocator platform_allocator_{
      memory::kEngineMemoryTagRender};

  Array<Buffer> staging_ssbo_camera_datas_{};
  Array<Buffer> ssbo_camera_datas_{};
  Array<Array<Buffer>> pending_buffer_destroys_{};
};

}  // namespace vk
}  // namespace rendering
}  // namespace comet

#endif  // COMET_RENDER_DRIVER_VULKAN_HANDLER_VULKAN_CAMERA_HANDLER_H_