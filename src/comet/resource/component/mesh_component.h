// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RESOURCE_COMPONENT_MESH_COMPONENT_H_
#define COMET_COMET_RESOURCE_COMPONENT_MESH_COMPONENT_H_

#include "comet_precompile.h"

#include "comet/resource/material_resource.h"
#include "comet/resource/model_resource.h"

namespace comet {
namespace resource {
struct MeshComponent {
  const resource::MeshResource* mesh{nullptr};
  const resource::MaterialResource* material{nullptr};
};
}  // namespace resource
}  // namespace comet

#endif  // COMET_COMET_RESOURCE_COMPONENT_MESH_COMPONENT_H_
