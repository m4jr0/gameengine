// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RESOURCE_HANDLER_SHADER_RESOURCE_HANDLER_H_
#define COMET_COMET_RESOURCE_HANDLER_SHADER_RESOURCE_HANDLER_H_

#include "comet/core/essentials.h"
#include "comet/core/type/array.h"
#include "comet/resource/handler/resource_handler.h"
#include "comet/resource/resource.h"
#include "comet/resource/shader_resource.h"

namespace comet {
namespace resource {
class ShaderResourceHandler : public ResourceHandler<ShaderResource> {
 public:
  ShaderResourceHandler(const ResourceHandlerDescr& descr);
  ShaderResourceHandler(const ShaderResourceHandler&) = delete;
  ShaderResourceHandler(ShaderResourceHandler&&) = delete;
  ShaderResourceHandler& operator=(const ShaderResourceHandler&) = delete;
  ShaderResourceHandler& operator=(ShaderResourceHandler&&) = delete;
  virtual ~ShaderResourceHandler() = default;

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
  void DumpVertexAttributes(const ShaderResourceDescr& descr, u8* buffer,
                            usize& cursor);
  void DumpUniforms(const ShaderResourceDescr& descr, u8* buffer,
                    usize& cursor);
  void DumpConstants(const ShaderResourceDescr& descr, u8* buffer,
                     usize& cursor);
  void DumpStorages(const ShaderResourceDescr& descr, u8* buffer,
                    usize& cursor);
  void ParseDescr(const Array<u8>& dumped_descr, ShaderResourceDescr& descr);
  void ParseShaderModules(const u8* buffer, ShaderResourceDescr& descr,
                          usize& cursor);
  void ParseShaderDefines(const u8* buffer, ShaderResourceDescr& descr,
                          usize& cursor);
  void ParseVertexAttributes(const u8* buffer, ShaderResourceDescr& descr,
                             usize& cursor);
  void ParseUniforms(const u8* buffer, ShaderResourceDescr& descr,
                     usize& cursor);
  void ParseConstants(const u8* buffer, ShaderResourceDescr& descr,
                      usize& cursor);
  void ParseStorages(const u8* buffer, ShaderResourceDescr& descr,
                     usize& cursor);
};
}  // namespace resource
}  // namespace comet

#endif  // COMET_COMET_RESOURCE_HANDLER_SHADER_RESOURCE_HANDLER_H_
