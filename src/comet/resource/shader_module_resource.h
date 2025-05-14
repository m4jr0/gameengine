// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RESOURCE_SHADER_MODULE_RESOURCE_H_
#define COMET_COMET_RESOURCE_SHADER_MODULE_RESOURCE_H_

#include "comet/core/essentials.h"
#include "comet/core/memory/allocator/allocator.h"
#include "comet/core/memory/allocator/platform_allocator.h"
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

class ShaderModuleHandler : public ResourceHandler {
 public:
  ShaderModuleHandler(memory::Allocator* loading_resources_allocator,
                      memory::Allocator* loading_resource_allocator);
  ShaderModuleHandler(const ShaderModuleHandler&) = delete;
  ShaderModuleHandler(ShaderModuleHandler&&) = delete;
  ShaderModuleHandler& operator=(const ShaderModuleHandler&) = delete;
  ShaderModuleHandler& operator=(ShaderModuleHandler&&) = delete;
  virtual ~ShaderModuleHandler() = default;

  void Initialize() override;
  void Shutdown() override;

 protected:
  ResourceFile Pack(memory::Allocator& allocator, const Resource& resource,
                    CompressionMode compression_mode) const override;
  Resource* Unpack(memory::Allocator& allocator,
                   const ResourceFile& file) override;

 private:
  // TODO(m4jr0): Use another allocator;
  memory::PlatformAllocator resource_data_allocator_{
      memory::kEngineMemoryTagResource};
};
}  // namespace resource
}  // namespace comet

#endif  // COMET_COMET_RESOURCE_SHADER_MODULE_RESOURCE_H_
