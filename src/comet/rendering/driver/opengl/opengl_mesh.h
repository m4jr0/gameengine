// Copyright 2022 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_OPENGL_MESH_H_
#define COMET_COMET_RENDERING_DRIVER_OPENGL_MESH_H_

#include "comet_precompile.h"

#include "comet/entity/component/mesh_component.h"
#include "comet/entity/entity.h"
#include "comet/rendering/driver/opengl/opengl_shader.h"

namespace comet {
namespace rendering {
namespace gl {
static ShaderProgram shader_program{"assets/shaders/model_shader.vs",
                                    "assets/shaders/model_shader.fs"};

struct MeshProxy {
  std::vector<entity::Vertex> vertices;
  std::vector<u32> indices;
  std::vector<resource::model::TextureTuple> texture_tuples;
  std::vector<u32> texture_ids;

  u32 vao{0};
  u32 vbo{0};
  u32 ebo{0};
};

MeshProxy& GenerateMeshProxy(entity::EntityId entity_id, entity::Mesh mesh,
                             const entity::Texture textures[],
                             uindex texture_count);
void DrawMeshProxy(MeshProxy proxy);
std::string GetTextureLabel(resource::texture::TextureType texture_type,
                            uindex texture_index);
bool IsMeshProxy(entity::EntityId entity_id);
void ClearMeshProxies();
void DrawMeshProxies();
void InitializeShader();
u32 LoadTexture(entity::Texture texture);

static std::unordered_map<entity::ComponentTypeId, MeshProxy> proxies;
static std::vector<u32> loaded_texture_ids;
static std::vector<resource::ResourceId> loaded_texture_resource_ids;
}  // namespace gl
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_OPENGL_MESH_H_
