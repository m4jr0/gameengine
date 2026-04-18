// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RESOURCE_SHADER_RESOURCE_H_
#define COMET_COMET_RESOURCE_SHADER_RESOURCE_H_

#include "comet/core/essentials.h"
#include "comet/core/type/array.h"
#include "comet/rendering/rendering_type.h"
#include "comet/resource/resource.h"
#include "comet/resource/resource_id.h"
#include "comet/resource/runtime/loaded_resource_handle.h"
#include "comet/resource/shader_module_resource.h"

namespace comet {
namespace resource {
struct ShaderResourceTag {};

using ShaderResourceId = ResourceIdT<ShaderResourceTag>;
using ShaderResourceHandle = LoadedResourceHandle<ShaderResourceTag>;

struct ShaderResourceDescr {
  rendering::RasterizerDescr rasterizer{};
  rendering::DepthStencilDescr depth_stencil{};
  rendering::PrimitiveTopology topology{rendering::PrimitiveTopology::Unknown};
  rendering::ShaderVertexLayout vertex_layout{
      rendering::ShaderVertexLayout::None};

  Array<ShaderModuleResourceId> shader_module_resource_ids{};
  Array<rendering::ShaderDefineDescr> defines{};
  Array<rendering::ShaderBindingDescr> bindings{};
  Array<rendering::ShaderPushConstantDescr> push_constants{};
};

struct ShaderResource : Resource {
  using Id = ShaderResourceId;
  using Handle = ShaderResourceHandle;
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

#endif  // COMET_COMET_RESOURCE_SHADER_RESOURCE_H_