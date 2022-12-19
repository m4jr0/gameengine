// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "vulkan_render_proxy_handler.h"

#include "comet/entity/component/mesh_component.h"
#include "comet/entity/component/transform_component.h"
#include "comet/rendering/driver/vulkan/utils/vulkan_buffer_utils.h"

namespace comet {
namespace rendering {
namespace vk {
RenderProxyHandler::RenderProxyHandler(const RenderProxyHandlerDescr& descr)
    : Handler{descr},
      material_handler_{descr.material_handler},
      mesh_handler_{descr.mesh_handler} {
  COMET_ASSERT(material_handler_ != nullptr, "Material handler is null!");
  COMET_ASSERT(mesh_handler_ != nullptr, "Mesh handler is null!");
}

void RenderProxyHandler::Shutdown() {
  update_frame_ = kInvalidFrameIndex;
  proxies_.clear();
  Handler::Shutdown();
}

RenderProxy* RenderProxyHandler::Get(uindex index) {
  auto* proxy{TryGet(index)};
  COMET_ASSERT(proxy != nullptr,
               "Requested render proxy with index does not exist: ", index,
               "!");
  return proxy;
}

RenderProxy* RenderProxyHandler::Get(const resource::MeshResource* resource) {
  auto* proxy{TryGet(resource)};
  COMET_ASSERT(proxy != nullptr,
               "Requested render proxy with resource ID does not exist: ",
               resource->resource_id, "!");
  return proxy;
}

RenderProxy* RenderProxyHandler::TryGet(uindex index) {
  COMET_ASSERT(index < proxies_.size(), "Requested render proxy at index #",
               index, ", but render proxy count is ", proxies_.size(), "!");
  return &proxies_[index];
}

RenderProxy* RenderProxyHandler::TryGet(
    const resource::MeshResource* resource) {
  for (uindex i{0}; i < proxies_.size(); ++i) {
    if (proxies_[i].mesh->id == mesh_handler_->GenerateMeshId(resource)) {
      return &proxies_[i];
    }
  }

  return nullptr;
}

void RenderProxyHandler::Update(time::Interpolation interpolation) {
  auto frame_count{context_->GetFrameCount()};

  if (update_frame_ == frame_count) {
    return;
  }

  // TODO(m4jr0): Remove temporary code.
  // Proxies should be managed with proper memory management and occlusion
  // culling.
  auto& entity_manager{Engine::Get().GetEntityManager()};

  const auto view{
      entity_manager
          .GetView<entity::MeshComponent, entity::TransformComponent>()};

  for (const auto entity_id : view) {
    auto* mesh_cmp{
        entity_manager.GetComponent<entity::MeshComponent>(entity_id)};
    auto* transform_cmp{
        entity_manager.GetComponent<entity::TransformComponent>(entity_id)};

    COMET_ASSERT(mesh_cmp->material != nullptr,
                 "Material bound to mesh component is null!");

    auto* proxy{TryGet(mesh_cmp->mesh)};

    if (proxy != nullptr) {
      proxy->transform += (transform_cmp->global - proxy->transform) *
                          static_cast<f32>(interpolation);
      continue;
    }

    Mesh* mesh{mesh_handler_->GetOrGenerate(mesh_cmp->mesh)};
    auto* material{material_handler_->GetOrGenerate(*mesh_cmp->material)};
    proxies_.push_back(GenerateInternal(
        *mesh, *material,
        transform_cmp->global * static_cast<f32>(interpolation)));
  }

  update_frame_ = frame_count;
}

RenderProxy RenderProxyHandler::GenerateInternal(Mesh& mesh, Material& material,
                                                 const glm::mat4& transform) {
  RenderProxy proxy{};
  proxy.mesh = &mesh;
  proxy.material = &material;
  proxy.transform = transform;
  return proxy;
}

void RenderProxyHandler::DrawProxies(const Shader& shader) {
  for (const auto& proxy : proxies_) {
    auto* material{proxy.material};
    COMET_ASSERT(material != nullptr, "Material should never be null!");
    material_handler_->UpdateInstance(*material, shader.id);

    MaterialLocalPacket local_packet{};
    local_packet.position = &proxy.transform;
    material_handler_->UpdateLocal(*material, local_packet, shader.id);
    Draw(proxy);
  }

  // Reset last drawn proxy to force rebinding meshes next frame.
  last_drawn_proxy_ = nullptr;
}

uindex RenderProxyHandler::GetCount() const noexcept { return proxies_.size(); }

void RenderProxyHandler::Draw(const RenderProxy& proxy) {
  auto command_buffer_handle{context_->GetFrameData().command_buffer_handle};

  if (&proxy != last_drawn_proxy_) {
    VkDeviceSize offset{0};
    vkCmdBindVertexBuffers(command_buffer_handle, 0, 1,
                           &proxy.mesh->vertex_buffer.handle, &offset);
    vkCmdBindIndexBuffer(
        command_buffer_handle, proxy.mesh->vertex_buffer.handle,
        static_cast<VkDeviceSize>(sizeof(Vertex) * proxy.mesh->vertices.size()),
        VK_INDEX_TYPE_UINT32);
    last_drawn_proxy_ = &proxy;
  }

  vkCmdDrawIndexed(command_buffer_handle,
                   static_cast<u32>(proxy.mesh->indices.size()), 1, 0, 0, 0);
}
}  // namespace vk
}  // namespace rendering
}  // namespace comet
