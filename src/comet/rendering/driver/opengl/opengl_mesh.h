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
// Temporary code to display something in OpenGL.
extern ShaderProgram& GetShaderProgram();

using TextureHandle = u32;
constexpr auto kInvalidTextureHandle{0};

using RepeatMode = u32;
constexpr auto kInvalidRepeatMode{static_cast<RepeatMode>(-1)};

using FilterMode = u32;
constexpr auto kInvalidFilterMode{static_cast<FilterMode>(-1)};

using VertexArrayObjectHandle = u32;
constexpr auto kInvalidVertexArrayObjectHandle{0};

using VertexBufferObjectHandle = u32;
constexpr auto kInvalidVertexBufferObjectHandle{0};

using ElementBufferObjectHandle = u32;
constexpr auto kInvalidElementBufferObjectHandle{0};

constexpr auto kMaxTextureLabelSize{32};

using TextureMapId = uindex;
constexpr auto kInvalidTextureMapId{static_cast<TextureMapId>(-1)};

struct TextureMap {
  TextureMapId id{kInvalidTextureMapId};
  TextureHandle texture_handle{kInvalidTextureHandle};
  schar texture_label[kMaxTextureLabelSize]{0};
  RepeatMode u_repeat_mode{kInvalidRepeatMode};
  RepeatMode v_repeat_mode{kInvalidRepeatMode};
  FilterMode min_filter_mode{kInvalidFilterMode};
  FilterMode mag_filter_mode{kInvalidFilterMode};
};

struct RenderProxy {
  VertexArrayObjectHandle vao{kInvalidVertexArrayObjectHandle};
  VertexBufferObjectHandle vbo{kInvalidVertexBufferObjectHandle};
  ElementBufferObjectHandle ebo{kInvalidElementBufferObjectHandle};
  glm::mat4 transform{};
  const std::vector<rendering::Vertex>* vertices{nullptr};
  const std::vector<u32>* indices{nullptr};
  const TextureMap* diffuse_map{nullptr};
  const TextureMap* specular_map{nullptr};
  const TextureMap* normal_map{nullptr};
};

RenderProxy& GenerateRenderProxy(entity::EntityId entity_id,
                                 const resource::MeshResource& mesh_resource,
                                 const glm::mat4& transform,
                                 const resource::MaterialResource& material,
                                 bool is_sampler_anisotropy = false);
void DrawRenderProxy(RenderProxy proxy);
void HandleTextureLabel(rendering::TextureType texture_type,
                        u32 texture_sub_index, schar* texture_label);
bool IsRenderProxy(entity::EntityId entity_id);
RenderProxy* TryGetRenderProxy(entity::EntityId entity_id);
void ClearRenderProxies();
TextureMapId GenerateTextureMapId(const resource::TextureMap& texture_map);
const TextureMap* GenerateTextureMap(const resource::TextureMap& texture_map,
                                     bool is_sampler_anisotropy = false);
void DrawRenderProxies();
FilterMode GetFilterMode(rendering::TextureFilterMode filter_mode);
RepeatMode GetRepeatMode(rendering::TextureRepeatMode repeat_mode);

static std::unordered_map<entity::ComponentTypeId, RenderProxy> proxies;
static std::vector<u32> loaded_texture_ids;
static std::unordered_map<TextureMapId, TextureMap> texture_maps;
}  // namespace gl
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_OPENGL_MESH_H_
