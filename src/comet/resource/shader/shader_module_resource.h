// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RESOURCE_SHADER_SHADER_MODULE_RESOURCE_H_
#define COMET_COMET_RESOURCE_SHADER_SHADER_MODULE_RESOURCE_H_

#include "comet/core/essentials.h"
#include "comet/core/type/array.h"
#include "comet/rendering/type/rendering_common_type.h"
#include "comet/rendering/type/rendering_shader_type.h"
#include "comet/resource/resource.h"
#include "comet/resource/resource_id.h"
#include "comet/resource/runtime/loaded_resource_handle.h"

namespace comet {
namespace resource {
struct ShaderModuleResourceTag {};
using ShaderModuleResourceId = ResourceIdT<ShaderModuleResourceTag>;
using ShaderModuleResourceHandle =
    LoadedResourceHandle<ShaderModuleResourceTag>;

struct ShaderModuleResourceDescr {
  rendering::ShaderStage stage{rendering::ShaderStage::Unknown};
  rendering::DriverType driver_type{rendering::DriverType::Unknown};
};

struct ShaderModuleResource : Resource {
  using Id = ShaderModuleResourceId;
  using Handle = ShaderModuleResourceHandle;
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

#endif  // COMET_COMET_RESOURCE_SHADER_SHADER_MODULE_RESOURCE_H_