// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_VULKAN_HANDLER_VULKAN_MESH_HANDLER_H_
#define COMET_COMET_RENDERING_DRIVER_VULKAN_HANDLER_VULKAN_MESH_HANDLER_H_

// External. ///////////////////////////////////////////////////////////////////
#include "vulkan/vulkan.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/essentials.h"
#include "comet/core/frame/frame_packet.h"
#include "comet/core/memory/allocator/free_list_allocator.h"
#include "comet/core/memory/memory.h"
#include "comet/core/memory/memory_utils.h"
#include "comet/core/type/array.h"
#include "comet/geometry/type/mesh.h"
#include "comet/rendering/driver/vulkan/handler/vulkan_handler.h"
#include "comet/rendering/driver/vulkan/type/vulkan_buffer.h"
#include "comet/rendering/driver/vulkan/type/vulkan_mesh.h"
#include "comet/rendering/driver/vulkan/type/vulkan_region_gpu_buffer.h"

namespace comet {
namespace rendering {
namespace vk {
namespace internal {
struct UpdateContext {
  VkDeviceSize new_vertex_size{0};
  VkDeviceSize new_index_size{0};
  VkDeviceSize dirty_vertex_size{0};
  VkDeviceSize dirty_index_size{0};
  VkDeviceSize total_vertex_size{0};
  VkDeviceSize total_index_size{0};
  VkDeviceSize current_staging_vertex_offset{0};
  VkDeviceSize current_staging_index_offset{0};
  frame::FrameArray<VkBufferCopy> vertex_copy_regions{};
  frame::FrameArray<VkBufferCopy> index_copy_regions{};
  VkCommandBuffer command_buffer_handle{VK_NULL_HANDLE};
  const QueueContext* upload_queue{nullptr};
  const QueueContext* graphics_queue{nullptr};
};

struct FreeRegion {
  static inline constexpr auto kInvalidOffset{static_cast<u32>(-1)};

  u32 offset{0};
  VkDeviceSize size{0};
};

struct MeshUploadResult {
  bool uploaded_vertex{false};
  bool uploaded_index{false};
};
}  // namespace internal

using MeshHandlerDescr = HandlerDescr;

class MeshHandler : public Handler {
 public:
  MeshHandler() = delete;
  explicit MeshHandler(const MeshHandlerDescr& descr);
  MeshHandler(const MeshHandler&) = delete;
  MeshHandler(MeshHandler&&) = delete;
  MeshHandler& operator=(const MeshHandler&) = delete;
  MeshHandler& operator=(MeshHandler&&) = delete;
  ~MeshHandler() override = default;

  void Update(const frame::FramePacket* packet);

  void Bind();
  void AcquireFromTransferQueueIfNeeded();

  void ReleasePendingUploadResources(FrameInFlightIndex frame_index);

  const MeshProxy* Get(geometry::MeshHandle handle) const;
  const MeshProxy* TryGet(geometry::MeshHandle handle) const;

 protected:
  void OnInitialize() override;
  void OnShutdown() override;

 private:
  static inline constexpr usize kVertexCountPerBlock_{2048};
  static inline constexpr usize kIndexCountPerBlock_{3 * kVertexCountPerBlock_};
  static inline constexpr usize kDefaultVertexCount_{
      memory::RoundUpToMultiple(3'000'000, kVertexCountPerBlock_)};
  static inline constexpr usize kDefaultIndexCount_{memory::RoundUpToMultiple(
      3 * kDefaultVertexCount_, kIndexCountPerBlock_)};
  static inline constexpr usize kDefaultStagingBufferSize_{
      kDefaultVertexCount_ * sizeof(geometry::SkinnedVertex) +
      kDefaultIndexCount_ * sizeof(geometry::Index)};

  internal::UpdateContext PrepareUpdate(const frame::FramePacket* packet);
  void FinishUpdate(internal::UpdateContext& update_context);

  void AddMeshProxies(const frame::AddedGeometries* geometry,
                      internal::UpdateContext& update_context);
  void UpdateMeshProxies(const frame::DirtyMeshes* meshes,
                         internal::UpdateContext& update_context);
  void DestroyMeshProxies(const frame::RemovedGeometries* geometry);

  internal::MeshUploadResult UploadMeshProxies(
      const internal::UpdateContext& update_context);

  static constexpr VkDeviceSize kMinUploadedBufferSize_{4194304};  // 4 MiB.
  static constexpr usize kDefaultProxyCount_{4096};
  static constexpr usize kDefaultReleaseBarrierCount_{2};
  static constexpr usize kDefaultAcquireBarrierCount_{2};

  Buffer staging_buffer_{};
  memory::FiberFreeListAllocator allocator_{
      sizeof(u32), sizeof(u32) * kDefaultProxyCount_ * 64,
      memory::kEngineMemoryTagRendering};
  Array<MeshProxy> proxies_{};
  VertexGpuBuffer vertex_buffer_{};
  IndexGpuBuffer index_buffer_{};
};
}  // namespace vk
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_VULKAN_HANDLER_VULKAN_MESH_HANDLER_H_