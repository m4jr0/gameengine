// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RESOURCE_SHADER_RESOURCE_H_
#define COMET_COMET_RESOURCE_SHADER_RESOURCE_H_

#include "comet_precompile.h"

#include "comet/rendering/driver/driver.h"
#include "comet/rendering/rendering_common.h"
#include "comet/resource/resource.h"
#include "comet/resource/shader_module_resource.h"

namespace comet {
namespace resource {
struct ShaderResourceDescr {
  bool is_wireframe{false};
  rendering::CullMode cull_mode{rendering::CullMode::Unknown};
  std::vector<std::string> shader_module_paths{};
  std::vector<rendering::ShaderVertexAttributeDescr> vertex_attributes{};
  std::vector<rendering::ShaderUniformDescr> uniforms{};
};

struct ShaderResource : Resource {
  static const ResourceTypeId kResourceTypeId;

  ShaderResourceDescr descr{};
};

class ShaderHandler : public ResourceHandler {
 public:
  ShaderHandler() = default;
  ShaderHandler(const ShaderHandler&) = delete;
  ShaderHandler(ShaderHandler&&) = delete;
  ShaderHandler& operator=(const ShaderHandler&) = delete;
  ShaderHandler& operator=(ShaderHandler&&) = delete;
  virtual ~ShaderHandler() = default;

 protected:
  ResourceFile Pack(const Resource& resource,
                    CompressionMode compression_mode) const override;
  std::unique_ptr<Resource> Unpack(const ResourceFile& file) const override;
  std::vector<u8> DumpDescr(const ShaderResourceDescr& descr) const;
  ShaderResourceDescr ParseDescr(const std::vector<u8>& dumped_descr) const;
};
}  // namespace resource
}  // namespace comet

#endif  // COMET_COMET_RESOURCE_SHADER_RESOURCE_H_
