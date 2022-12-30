// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_OPENGL_MESH_H_
#define COMET_COMET_RENDERING_DRIVER_OPENGL_MESH_H_

#include "comet_precompile.h"

#include "glm/glm.hpp"

#include "comet/entity/component/component.h"
#include "comet/entity/entity.h"
#include "comet/rendering/driver/opengl/opengl_shader.h"
#include "comet/rendering/rendering_common.h"
#include "comet/resource/model_resource.h"
#include "comet/resource/texture_resource.h"

namespace comet {
namespace rendering {
namespace gl {
extern ShaderProgram& GetShaderProgram();

struct RenderProxy {
  std::vector<rendering::Vertex> vertices;
  std::vector<u32> indices;
  std::vector<resource::TextureTuple> texture_tuples;
  std::vector<u32> texture_ids;
  glm::mat4 transform;

  u32 vao{0};
  u32 vbo{0};
  u32 ebo{0};
};

RenderProxy& GenerateRenderProxy(entity::EntityId entity_id,
                                 const resource::MeshResource& mesh_resource,
                                 const glm::mat4& transform,
                                 const resource::MaterialResource& material);
void DrawRenderProxy(RenderProxy proxy);
std::string GetTextureLabel(rendering::TextureType texture_type,
                            uindex texture_index);
bool IsRenderProxy(entity::EntityId entity_id);
RenderProxy* TryGetRenderProxy(entity::EntityId entity_id);
void ClearRenderProxies();
void DrawRenderProxies();
void InitializeShader();
u32 LoadTexture(const resource::TextureResource* resource);

static std::unordered_map<entity::ComponentTypeId, RenderProxy> proxies;
static std::vector<u32> loaded_texture_ids;
static std::vector<resource::ResourceId> loaded_texture_resource_ids;
}  // namespace gl
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_OPENGL_MESH_H_
