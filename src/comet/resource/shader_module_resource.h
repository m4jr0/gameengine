// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RESOURCE_SHADER_MODULE_RESOURCE_H_
#define COMET_COMET_RESOURCE_SHADER_MODULE_RESOURCE_H_

#include "comet_precompile.h"

#include "comet/rendering/driver/driver.h"
#include "comet/rendering/rendering_common.h"
#include "comet/resource/resource.h"
#include "comet/resource/shader_module_resource.h"

namespace comet {
namespace resource {
struct ShaderModuleResourceDescr {
  rendering::ShaderModuleType shader_type{rendering::ShaderModuleType::Unknown};
  rendering::DriverType driver_type{rendering::DriverType::Unknown};
};

struct ShaderModuleResource : Resource {
  static const ResourceTypeId kResourceTypeId;

  ShaderModuleResourceDescr descr{};
  std::vector<u8> data{};
};

class ShaderModuleHandler : public ResourceHandler {
 public:
  ShaderModuleHandler() = default;
  ShaderModuleHandler(const ShaderModuleHandler&) = delete;
  ShaderModuleHandler(ShaderModuleHandler&&) = delete;
  ShaderModuleHandler& operator=(const ShaderModuleHandler&) = delete;
  ShaderModuleHandler& operator=(ShaderModuleHandler&&) = delete;
  virtual ~ShaderModuleHandler() = default;

 protected:
  ResourceFile Pack(const Resource& resource,
                    CompressionMode compression_mode) const override;
  std::unique_ptr<Resource> Unpack(const ResourceFile& file) const override;
};
}  // namespace resource
}  // namespace comet

#endif  // COMET_COMET_RESOURCE_SHADER_MODULE_RESOURCE_H_
