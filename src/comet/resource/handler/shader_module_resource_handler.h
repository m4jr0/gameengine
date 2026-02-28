// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RESOURCE_HANDLER_SHADER_MODULE_RESOURCE_HANDLER_H_
#define COMET_COMET_RESOURCE_HANDLER_SHADER_MODULE_RESOURCE_HANDLER_H_

#include "comet/core/essentials.h"
#include "comet/resource/handler/resource_handler.h"
#include "comet/resource/resource.h"
#include "comet/resource/shader_module_resource.h"

namespace comet {
namespace resource {
class ShaderModuleResourceHandler
    : public ResourceHandler<ShaderModuleResource> {
 public:
  ShaderModuleResourceHandler(const ResourceHandlerDescr& descr);
  ShaderModuleResourceHandler(const ShaderModuleResourceHandler&) = delete;
  ShaderModuleResourceHandler(ShaderModuleResourceHandler&&) = delete;
  ShaderModuleResourceHandler& operator=(const ShaderModuleResourceHandler&) =
      delete;
  ShaderModuleResourceHandler& operator=(ShaderModuleResourceHandler&&) =
      delete;
  virtual ~ShaderModuleResourceHandler() = default;

  ResourceFile Pack(const ShaderModuleResource& resource,
                    CompressionMode compression_mode) override;
  void Unpack(const ResourceFile& file, ResourceLifeSpan life_span,
              ShaderModuleResource* resource) override;
};
}  // namespace resource
}  // namespace comet

#endif  // COMET_COMET_RESOURCE_HANDLER_SHADER_MODULE_RESOURCE_HANDLER_H_
