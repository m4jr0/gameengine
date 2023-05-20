// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "opengl_render_proxy_handler.h"

#include "comet/entity/entity_manager.h"
#include "comet/physics/component/transform_component.h"
#include "comet/rendering/camera/camera_manager.h"
#include "comet/rendering/driver/vulkan/utils/vulkan_buffer_utils.h"
#include "comet/resource/component/mesh_component.h"

namespace comet {
namespace rendering {
namespace gl {
RenderProxyHandler::RenderProxyHandler(const RenderProxyHandlerDescr& descr)
    : Handler{descr},
      material_handler_{descr.material_handler},
      mesh_handler_{descr.mesh_handler},
      shader_handler_{descr.shader_handler} {
  COMET_ASSERT(material_handler_ != nullptr, "Material handler is null!");
  COMET_ASSERT(mesh_handler_ != nullptr, "Mesh handler is null!");
  COMET_ASSERT(shader_handler_ != nullptr, "Shader handler is null!");
}

void RenderProxyHandler::Shutdown() {
  update_frame_ = kInvalidFrameIndex;
  proxies_.clear();
  Handler::Shutdown();
}

void RenderProxyHandler::Update(FrameIndex frame_count,
                                time::Interpolation interpolation) {
  if (update_frame_ == frame_count) {
    return;
  }

  const auto& frustum{
      rendering::CameraManager::Get().GetMainCamera()->GetFrustum()};
  proxies_.clear();

  // Reset proxies_ to its default capacity;
  if (proxies_.size() != kDefaultProxyCount) {
    proxies_.reserve(kDefaultProxyCount);
  }

  auto& entity_manager{entity::EntityManager::Get()};

  entity_manager.Each<resource::MeshComponent, physics::TransformComponent>(
      [&](auto entity_id) {
        auto* mesh_cmp{
            entity_manager.GetComponent<resource::MeshComponent>(entity_id)};
        auto* transform_cmp{
            entity_manager.GetComponent<physics::TransformComponent>(
                entity_id)};

        const math::Aabb aabb{math::GenerateGlobalAabb(
            mesh_cmp->mesh->local_center, mesh_cmp->mesh->local_max_extents,
            transform_cmp->global)};

        if (!frustum.IsAabbContained(aabb)) {
          return;
        }

        COMET_ASSERT(mesh_cmp->material != nullptr,
                     "Material bound to mesh component is null!");

        Mesh* mesh{mesh_handler_->GetOrGenerate(mesh_cmp->mesh)};
        auto* material{material_handler_->GetOrGenerate(*mesh_cmp->material)};
        proxies_.push_back(
            GenerateInternal(*mesh, *material, transform_cmp->global));
      });

  update_frame_ = frame_count;
}

RenderProxy RenderProxyHandler::GenerateInternal(Mesh& mesh, Material& material,
                                                 const math::Mat4& transform) {
  RenderProxy proxy{};
  proxy.mesh = &mesh;
  proxy.material = &material;
  proxy.transform = transform;
  return proxy;
}

void RenderProxyHandler::DrawProxies(FrameIndex frame_count, Shader& shader) {
  for (const auto& proxy : proxies_) {
    auto* material{proxy.material};
    COMET_ASSERT(material != nullptr, "Material should never be null!");
    material_handler_->UpdateInstance(*material, shader.id, frame_count);

    ShaderLocalPacket local_packet{};
    local_packet.position = &proxy.transform;
    shader_handler_->UpdateLocal(shader, local_packet);
    Draw(frame_count, proxy);
  }

  // Reset last drawn proxy to force rebinding meshes next frame.
  last_drawn_mesh_ = nullptr;
}

void RenderProxyHandler::DrawProxiesForDebugging(FrameIndex frame_count,
                                                 Shader& shader) {
  for (const auto& proxy : proxies_) {
    ShaderLocalPacket local_packet{};
    local_packet.position = &proxy.transform;
    shader_handler_->UpdateLocal(shader, local_packet);
    Draw(frame_count, proxy);
  }

  // Reset last drawn proxy to force rebinding meshes next frame.
  last_drawn_mesh_ = nullptr;
}

u32 RenderProxyHandler::GetDrawCount() const noexcept {
  return proxies_.size();
}

void RenderProxyHandler::Draw(FrameIndex frame_count,
                              const RenderProxy& proxy) {
  mesh_handler_->Bind(proxy.mesh);
  glDrawElements(GL_TRIANGLES, proxy.mesh->indices.size(), GL_UNSIGNED_INT, 0);
}
}  // namespace gl
}  // namespace rendering
}  // namespace comet
