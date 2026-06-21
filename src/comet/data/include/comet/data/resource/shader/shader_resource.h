// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_DATA_RESOURCE_SHADER_SHADER_RESOURCE_H_
#define COMET_DATA_RESOURCE_SHADER_SHADER_RESOURCE_H_

#include "comet/core/essentials.h"
#include "comet/core/container/array.h"
#include "comet/data/render/pipeline.h"
#include "comet/data/render/shader.h"
#include "comet/data/resource/resource.h"
#include "comet/data/resource/resource_file.h"
#include "comet/data/resource/resource_id.h"
#include "comet/data/resource/shader/shader_module_resource.h"

namespace comet {
namespace resource {
struct ShaderResourceTag {};

using ShaderResourceId = ResourceIdT<ShaderResourceTag>;

struct ShaderResourceDescr {
  render::RasterizerDescr rasterizer{};
  render::DepthStencilDescr depth_stencil{};
  render::PrimitiveTopology topology{render::PrimitiveTopology::Unknown};
  render::ShaderVertexLayout vertex_layout{
      render::ShaderVertexLayout::None};

  Array<ShaderModuleResourceId> shader_module_resource_ids{};
  Array<render::ShaderDefineDescr> defines{};
  Array<render::ShaderBindingDescr> bindings{};
  Array<render::ShaderPushConstantDescr> push_constants{};
};

struct ShaderResource : Resource {
  using Id = ShaderResourceId;
  using TypeId = ResourceTypeId;
  using TypeName = ResourceTypeName;

  static constexpr TypeName kResourceTypeName{"shader"};
  static const TypeId kResourceTypeId;

  ShaderResourceDescr descr{};

  Id GetId() const noexcept;
};

usize GetSizeFromDescr(const ShaderResourceDescr& descr);

const schar** GetActiveShaderEngineDefines(usize& count);
bool IsShaderEngineDefineSet(const schar* engine_define,
                             usize engine_define_len);
}  // namespace resource
}  // namespace comet

#endif  // COMET_DATA_RESOURCE_SHADER_SHADER_RESOURCE_H_