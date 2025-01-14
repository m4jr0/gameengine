// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RESOURCE_SHADER_RESOURCE_H_
#define COMET_COMET_RESOURCE_SHADER_RESOURCE_H_

#include "comet/core/essentials.h"
#include "comet/core/memory/allocator/allocator.h"
#include "comet/core/type/array.h"
#include "comet/core/type/tstring.h"
#include "comet/rendering/driver/driver.h"
#include "comet/rendering/rendering_common.h"
#include "comet/resource/resource.h"
#include "comet/resource/shader_module_resource.h"

namespace comet {
namespace resource {
struct ShaderResourceDescr {
  bool is_wireframe{false};
  rendering::CullMode cull_mode{rendering::CullMode::Unknown};
  Array<TString> shader_module_paths{};
  Array<rendering::ShaderVertexAttributeDescr> vertex_attributes{};
  Array<rendering::ShaderUniformDescr> uniforms{};
  Array<rendering::ShaderConstantDescr> constants{};
  Array<rendering::ShaderStorageDescr> storages{};
};

struct ShaderResource : Resource {
  static const ResourceTypeId kResourceTypeId;

  ShaderResourceDescr descr{};
};

class ShaderHandler : public ResourceHandler {
 public:
  ShaderHandler(memory::Allocator* loading_resources_allocator,
                memory::Allocator* loading_resource_allocator);
  ShaderHandler(const ShaderHandler&) = delete;
  ShaderHandler(ShaderHandler&&) = delete;
  ShaderHandler& operator=(const ShaderHandler&) = delete;
  ShaderHandler& operator=(ShaderHandler&&) = delete;
  virtual ~ShaderHandler() = default;

 protected:
  ResourceFile Pack(memory::Allocator& allocator, const Resource& resource,
                    CompressionMode compression_mode) const override;
  Resource* Unpack(memory::Allocator& allocator,
                   const ResourceFile& file) override;

 private:
  static Array<u8> DumpDescr(memory::Allocator& allocator,
                             const ShaderResourceDescr& descr);
  static usize GetSizeFromDescr(const ShaderResourceDescr& descr);
  static void DumpShaderModules(const ShaderResourceDescr& descr, u8* buffer,
                                usize& cursor);
  static void DumpVertexAttributes(const ShaderResourceDescr& descr, u8* buffer,
                                   usize& cursor);
  static void DumpUniforms(const ShaderResourceDescr& descr, u8* buffer,
                           usize& cursor);
  static void DumpConstants(const ShaderResourceDescr& descr, u8* buffer,
                            usize& cursor);
  static void DumpStorages(const ShaderResourceDescr& descr, u8* buffer,
                           usize& cursor);
  static void ParseDescr(const Array<u8>& dumped_descr,
                         memory::Allocator* allocator,
                         ShaderResourceDescr& descr);
  static void ParseShaderModules(const u8* buffer, ShaderResourceDescr& descr,
                                 usize& cursor);
  static void ParseVertexAttributes(const u8* buffer,
                                    ShaderResourceDescr& descr, usize& cursor);
  static void ParseUniforms(const u8* buffer, ShaderResourceDescr& descr,
                            usize& cursor);
  static void ParseConstants(const u8* buffer, ShaderResourceDescr& descr,
                             usize& cursor);
  static void ParseStorages(const u8* buffer, memory::Allocator* allocator,
                            ShaderResourceDescr& descr, usize& cursor);
};
}  // namespace resource
}  // namespace comet

#endif  // COMET_COMET_RESOURCE_SHADER_RESOURCE_H_
