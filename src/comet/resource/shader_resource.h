// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RESOURCE_SHADER_RESOURCE_H_
#define COMET_COMET_RESOURCE_SHADER_RESOURCE_H_

#include "comet/core/essentials.h"
#include "comet/core/type/array.h"
#include "comet/core/type/tstring.h"
#include "comet/rendering/rendering_common.h"
#include "comet/resource/resource.h"

namespace comet {
namespace resource {
struct ShaderResourceDescr {
  rendering::RasterizerDescr rasterizer{};
  rendering::DepthStencilDescr depth_stencil{};
  rendering::PrimitiveTopology topology{rendering::PrimitiveTopology::Unknown};
  rendering::ShaderVertexLayout vertex_layout{
      rendering::ShaderVertexLayout::None};

  Array<TString> shader_module_paths{};
  Array<rendering::ShaderDefineDescr> defines{};
  Array<rendering::ShaderBindingDescr> bindings{};
  Array<rendering::ShaderPushConstantDescr> push_constants{};
};

struct ShaderResource : Resource {
  static const ResourceTypeId kResourceTypeId;

  ShaderResourceDescr descr{};
};

usize GetSizeFromDescr(const ShaderResourceDescr& descr);

const schar** GetActiveShaderEngineDefines(usize& count);
bool IsShaderEngineDefineSet(const schar* engine_define,
                             usize engine_define_len);
ResourceId GetDefaultShaderResourceId();
}  // namespace resource
}  // namespace comet

#endif  // COMET_COMET_RESOURCE_SHADER_RESOURCE_H_