// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_DATA_RESOURCE_SHADER_SHADER_MODULE_RESOURCE_H_
#define COMET_DATA_RESOURCE_SHADER_SHADER_MODULE_RESOURCE_H_

#include "comet/core/essentials.h"
#include "comet/core/container/array.h"
#include "comet/render/common.h"
#include "comet/data/render/shader.h"
#include "comet/data/resource/resource_file.h"
#include "comet/data/resource/resource.h"
#include "comet/data/resource/resource_id.h"

namespace comet {
namespace resource {
struct ShaderModuleResourceTag {};
using ShaderModuleResourceId = ResourceIdT<ShaderModuleResourceTag>;

struct ShaderModuleResourceDescr {
  render::ShaderStage stage{render::ShaderStage::Unknown};
  render::DriverType driver_type{render::DriverType::Unknown};
};

struct ShaderModuleResource : Resource {
  using Id = ShaderModuleResourceId;
  using TypeId = ResourceTypeId;
  using TypeName = ResourceTypeName;

  static constexpr TypeName kResourceTypeName{"shader_module"};
  static const TypeId kResourceTypeId;

  ShaderModuleResourceDescr descr{};
  Array<u8> data{};

  Id GetId() const noexcept;
};

usize GetShaderModuleResourceSize(const ShaderModuleResource& resource);
}  // namespace resource
}  // namespace comet

#endif  // COMET_DATA_RESOURCE_SHADER_SHADER_MODULE_RESOURCE_H_