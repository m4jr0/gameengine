// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RESOURCE_SHADER_MODULE_RESOURCE_H_
#define COMET_COMET_RESOURCE_SHADER_MODULE_RESOURCE_H_

#include "comet/core/essentials.h"
#include "comet/rendering/rendering_common.h"
#include "comet/resource/resource.h"

namespace comet {
namespace resource {
struct ShaderModuleResourceDescr {
  rendering::ShaderModuleType shader_type{rendering::ShaderModuleType::Unknown};
  rendering::DriverType driver_type{rendering::DriverType::Unknown};
};

struct ShaderModuleResource : Resource {
  static const ResourceTypeId kResourceTypeId;

  ShaderModuleResourceDescr descr{};
  Array<u8> data{};
};
}  // namespace resource
}  // namespace comet

#endif  // COMET_COMET_RESOURCE_SHADER_MODULE_RESOURCE_H_
