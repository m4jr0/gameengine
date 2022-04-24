// Copyright 2022 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_ENTITY_COMPONENT_MESH_COMPONENT_H_
#define COMET_COMET_ENTITY_COMPONENT_MESH_COMPONENT_H_

#include "comet_precompile.h"

#include "comet/entity/component/component.h"
#include "comet/resource/model_resource.h"

namespace comet {
namespace entity {
using Vertex = resource::model::Vertex;
using Index = resource::model::Index;
using Model = const resource::model::ModelResource*;
using Mesh = const resource::model::MeshResource*;
using Texture = const resource::texture::TextureResource*;

constexpr auto kTextureCount{5};

struct MeshComponent {
  static const ComponentTypeId kComponentTypeId;

  Mesh mesh{nullptr};
  uindex texture_count{0};
  Texture textures[kTextureCount];
};
}  // namespace entity
}  // namespace comet

#endif  // COMET_COMET_ENTITY_COMPONENT_MESH_COMPONENT_H_
