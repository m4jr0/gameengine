// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "vulkan_mesh_handler.h"

#include "comet/rendering/driver/vulkan/data/vulkan_buffer.h"
#include "comet/rendering/driver/vulkan/utils/vulkan_buffer_utils.h"
#include "comet/rendering/rendering_common.h"

namespace comet {
namespace rendering {
namespace vk {
MeshHandler::MeshHandler(const MeshHandlerDescr& descr) : Handler{descr} {}

void MeshHandler::Initialize() {
  Handler::Initialize();
  InitializeStagingBuffer();
}

void MeshHandler::Shutdown() {
  for (auto& it : mesh_proxies_) {
    Destroy(it.second, true);
  }

  mesh_proxies_.clear();
  DestroyStagingBuffer();
  Handler::Shutdown();
}

MeshProxy* MeshHandler::Generate(geometry::Mesh* mesh) {
  MeshProxy proxy{};
  proxy.id = mesh->id;
  proxy.mesh = mesh;
  proxy.vertex_buffer.allocator_handle = context_->GetAllocatorHandle();
  return Register(proxy);
}

MeshProxy* MeshHandler::Get(geometry::MeshId proxy_id) {
  auto* proxy{TryGet(proxy_id)};
  COMET_ASSERT(proxy != nullptr,
               "Requested mesh proxy does not exist: ", proxy_id, "!");
  return proxy;
}

MeshProxy* MeshHandler::Get(const geometry::Mesh* mesh) {
  return Get(mesh->id);
}

MeshProxy* MeshHandler::TryGet(geometry::MeshId proxy_id) {
  auto it{mesh_proxies_.find(proxy_id)};

  if (it != mesh_proxies_.end()) {
    return &it->second;
  }

  return nullptr;
}

MeshProxy* MeshHandler::TryGet(const geometry::Mesh* mesh) {
  return TryGet(mesh->id);
}

MeshProxy* MeshHandler::GetOrGenerate(geometry::Mesh* mesh) {
  auto* proxy{TryGet(mesh)};

  if (proxy != nullptr) {
    return proxy;
  }

  return Generate(mesh);
}

void MeshHandler::Destroy(geometry::MeshId proxy_id) {
  Destroy(*Get(proxy_id));
}

void MeshHandler::Destroy(MeshProxy& proxy) { Destroy(proxy, false); }

void MeshHandler::Update(geometry::MeshId proxy_id) { Update(*Get(proxy_id)); }

void MeshHandler::Update(MeshProxy& proxy) {
  if (!proxy.mesh->is_dirty) {
    return;
  }

  Upload(proxy);
}

void MeshHandler::Bind(geometry::MeshId proxy_id) { Bind(Get(proxy_id)); }

void MeshHandler::Bind(const MeshProxy* proxy) {
  auto command_buffer_handle{context_->GetFrameData().command_buffer_handle};
  const auto vertex_buffer_size{static_cast<VkDeviceSize>(
      proxy->mesh->vertices.size() * sizeof(geometry::Vertex))};
  VkDeviceSize offset{0};
  vkCmdBindVertexBuffers(command_buffer_handle, 0, 1,
                         &proxy->vertex_buffer.handle, &offset);
  vkCmdBindIndexBuffer(command_buffer_handle, proxy->vertex_buffer.handle,
                       static_cast<VkDeviceSize>(vertex_buffer_size),
                       VK_INDEX_TYPE_UINT32);
}

void MeshHandler::InitializeStagingBuffer() {
  staging_buffer_ = GenerateBuffer(
      context_->GetAllocatorHandle(), kStagingBufferSize,
      VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
      VMA_MEMORY_USAGE_AUTO,
      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
          VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
      VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
      VK_SHARING_MODE_EXCLUSIVE, "mesh_stating_buffer");
}

void MeshHandler::DestroyStagingBuffer() { DestroyBuffer(staging_buffer_); }

MeshProxy* MeshHandler::Register(MeshProxy& proxy) {
#ifdef COMET_DEBUG
  const auto proxy_id{proxy.id};
#endif  // COMET_DEBUG

  const auto insert_pair{mesh_proxies_.emplace(proxy.id, std::move(proxy))};
  MeshProxy* to_return{insert_pair.second ? &insert_pair.first->second
                                          : nullptr};

  if (to_return != nullptr) {
    Upload(*to_return);
  } else {
    COMET_ASSERT(false, "Could not insert mesh proxy #", proxy_id, "!");
  }

  return to_return;
}

void MeshHandler::Destroy(MeshProxy& proxy, bool is_destroying_handler) {
  if (proxy.vertex_buffer.allocation_handle != VK_NULL_HANDLE) {
    DestroyBuffer(proxy.vertex_buffer);
  }

  proxy.vertex_buffer = {};
  proxy.mesh = nullptr;

  if (!is_destroying_handler) {
    mesh_proxies_.erase(proxy.id);
  }

  proxy.id = geometry::kInvalidMeshId;
}

void MeshHandler::Upload(MeshProxy& proxy) {
  const auto vertex_buffer_size{static_cast<VkDeviceSize>(
      proxy.mesh->vertices.size() * sizeof(geometry::Vertex))};
  const auto index_buffer_size{
      static_cast<u32>(proxy.mesh->indices.size() * sizeof(geometry::Index))};

  const auto buffer_size{vertex_buffer_size + index_buffer_size};

  MapBuffer(staging_buffer_);
  CopyToBuffer(staging_buffer_, proxy.mesh->vertices.data(),
               vertex_buffer_size);
  CopyToBuffer(staging_buffer_, proxy.mesh->indices.data(), index_buffer_size,
               vertex_buffer_size);
  UnmapBuffer(staging_buffer_);

  if (proxy.vertex_buffer.handle == VK_NULL_HANDLE) {
    proxy.vertex_buffer = GenerateBuffer(
        context_->GetAllocatorHandle(), buffer_size,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT |
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
            VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        VMA_MEMORY_USAGE_AUTO, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 0,
        VK_SHARING_MODE_EXCLUSIVE, "vertex_buffer");
  }

  CopyBuffer(context_->GetDevice(),
             context_->GetUploadContext().command_pool_handle, staging_buffer_,
             proxy.vertex_buffer, buffer_size);
  proxy.mesh->is_dirty = false;
}
}  // namespace vk
}  // namespace rendering
}  // namespace comet
