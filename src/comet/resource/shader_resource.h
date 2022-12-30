// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RESOURCE_SHADER_RESOURCE_H_
#define COMET_COMET_RESOURCE_SHADER_RESOURCE_H_

#include "comet_precompile.h"

#include "comet/rendering/driver/driver.h"
#include "comet/resource/resource.h"
#include "comet/resource/shader_resource.h"

namespace comet {
namespace resource {
enum class ShaderType : u8 { Unknown = 0, Vertex, Fragment };

struct ShaderResourceDescr {
  ShaderType shader_type{ShaderType::Unknown};
  rendering::DriverType driver_type{rendering::DriverType::Unknown};
};

struct ShaderResource : Resource {
  static const ResourceTypeId kResourceTypeId;

  ShaderResourceDescr descr;
  std::vector<u8> data;
};

class ShaderHandler : public ResourceHandler {
 public:
  ShaderHandler() = default;
  ShaderHandler(const ShaderHandler&) = delete;
  ShaderHandler(ShaderHandler&&) = delete;
  ShaderHandler& operator=(const ShaderHandler&) = delete;
  ShaderHandler& operator=(ShaderHandler&&) = delete;
  ~ShaderHandler() = default;

 protected:
  ResourceFile Pack(const Resource& resource,
                    CompressionMode compression_mode) const override;
  std::unique_ptr<Resource> Unpack(const ResourceFile& file) const override;
};
}  // namespace resource
}  // namespace comet

#endif  // COMET_COMET_RESOURCE_SHADER_RESOURCE_H_
