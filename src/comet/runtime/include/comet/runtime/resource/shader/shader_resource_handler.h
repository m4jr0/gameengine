// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_RUNTIME_RESOURCE_SHADER_SHADER_RESOURCE_HANDLER_H_
#define COMET_RUNTIME_RESOURCE_SHADER_SHADER_RESOURCE_HANDLER_H_

#include "comet/core/essentials.h"
#include "comet/core/container/array.h"
#include "comet/data/resource/resource_file.h"
#include "comet/runtime/resource/resource_handler.h"
#include "comet/data/resource/resource.h"
#include "comet/data/resource/shader/shader_resource.h"
#include "comet/data/resource/common.h"

namespace comet {
namespace resource {
class ShaderResourceHandler
    : public ResourceHandler<ShaderResourceTag, ShaderResource> {
 public:
  using Base = ResourceHandler;

  explicit ShaderResourceHandler(const ResourceHandlerDescr& descr);
  ShaderResourceHandler(const ShaderResourceHandler&) = delete;
  ShaderResourceHandler(ShaderResourceHandler&&) = delete;
  ShaderResourceHandler& operator=(const ShaderResourceHandler&) = delete;
  ShaderResourceHandler& operator=(ShaderResourceHandler&&) = delete;
  ~ShaderResourceHandler() override = default;

  ResourceFile Pack(const ShaderResource& resource,
                    CompressionMode compression_mode) override;
  void Unpack(const ResourceFile& file, ResourceLifeSpan life_span,
              ShaderResource* resource) override;

 private:
  Array<u8> DumpDescr(const ShaderResourceDescr& descr);

  void DumpShaderModules(const ShaderResourceDescr& descr, u8* buffer,
                         usize& cursor);
  void DumpShaderDefines(const ShaderResourceDescr& descr, u8* buffer,
                         usize& cursor);
  void DumpBindings(const ShaderResourceDescr& descr, u8* buffer,
                    usize& cursor);
  void DumpPushConstants(const ShaderResourceDescr& descr, u8* buffer,
                         usize& cursor);

  void ParseDescr(const Array<u8>& dumped_descr, ShaderResourceDescr& descr);

  void ParseShaderModules(const u8* buffer, ShaderResourceDescr& descr,
                          usize& cursor);
  void ParseShaderDefines(const u8* buffer, ShaderResourceDescr& descr,
                          usize& cursor);
  void ParseBindings(const u8* buffer, ShaderResourceDescr& descr,
                     usize& cursor);
  void ParsePushConstants(const u8* buffer, ShaderResourceDescr& descr,
                          usize& cursor);
};
}  // namespace resource
}  // namespace comet

#endif  // COMET_RUNTIME_RESOURCE_SHADER_SHADER_RESOURCE_HANDLER_H_