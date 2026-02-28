// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RESOURCE_HANDLER_MATERIAL_RESOURCE_HANDLER_H_
#define COMET_COMET_RESOURCE_HANDLER_MATERIAL_RESOURCE_HANDLER_H_

#include "comet/core/essentials.h"
#include "comet/core/memory/memory.h"
#include "comet/resource/handler/resource_handler.h"
#include "comet/resource/material_resource.h"
#include "comet/resource/resource.h"

namespace comet {
namespace resource {
class MaterialResourceHandler : public ResourceHandler<MaterialResource> {
 public:
  MaterialResourceHandler(const ResourceHandlerDescr& descr);
  MaterialResourceHandler(const MaterialResourceHandler&) = delete;
  MaterialResourceHandler(MaterialResourceHandler&&) = delete;
  MaterialResourceHandler& operator=(const MaterialResourceHandler&) = delete;
  MaterialResourceHandler& operator=(MaterialResourceHandler&&) = delete;
  virtual ~MaterialResourceHandler() = default;

  void InitializeDefaults() override;
  void DestroyDefaults() override;

  ResourceFile Pack(const MaterialResource& resource,
                    CompressionMode compression_mode) override;
  void Unpack(const ResourceFile& file, ResourceLifeSpan life_span,
              MaterialResource* resource) override;

  MaterialResource* GetDefaultMaterialResource();

 private:
  memory::UniquePtr<MaterialResource> default_material_{nullptr};
};
}  // namespace resource
}  // namespace comet

#endif  // COMET_COMET_RESOURCE_HANDLER_MATERIAL_RESOURCE_HANDLER_H_
