// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_OPENGL_HANDLER_OPENGL_MATERIAL_HANDLER_H_
#define COMET_COMET_RENDERING_DRIVER_OPENGL_HANDLER_OPENGL_MATERIAL_HANDLER_H_

#include "comet/core/essentials.h"
#include "comet/core/memory/allocator/free_list_allocator.h"
#include "comet/core/type/map.h"
#include "comet/rendering/driver/opengl/data/opengl_material.h"
#include "comet/rendering/driver/opengl/handler/opengl_texture_handler.h"
#include "comet/resource/material_resource.h"
#include "comet/resource/resource.h"

namespace comet {
namespace rendering {
namespace gl {
struct MaterialHandlerDescr : HandlerDescr {
  TextureHandler* texture_handler{nullptr};
};

class MaterialHandler : public Handler {
 public:
  MaterialHandler() = delete;
  explicit MaterialHandler(const MaterialHandlerDescr& descr);
  MaterialHandler(const MaterialHandler&) = delete;
  MaterialHandler(MaterialHandler&&) = delete;
  MaterialHandler& operator=(const MaterialHandler&) = delete;
  MaterialHandler& operator=(MaterialHandler&&) = delete;
  virtual ~MaterialHandler() = default;

  void Initialize() override;
  void Shutdown() override;

  Material* Generate(const MaterialDescr& descr);
  Material* Generate(const resource::MaterialResource* resource);
  Material* Get(MaterialId material_id);
  Material* TryGet(MaterialId material_id);
  Material* GetOrGenerate(const MaterialDescr& descr);
  Material* GetOrGenerate(const resource::MaterialResource* resource);
  void Destroy(MaterialId material_id);
  void Destroy(Material* material);

 private:
  TextureMap GenerateTextureMap(const resource::TextureMap* map,
                                resource::ResourceLifeSpan life_span =
                                    resource::ResourceLifeSpan::Manual);
  void Destroy(Material* material, bool is_destroying_handler);
  TextureType GetTextureType(rendering::TextureType texture_type);
  RepeatMode GetRepeatMode(TextureRepeatMode repeat_mode);
  FilterMode GetFilterMode(TextureFilterMode filter_mode);

  memory::FiberFreeListAllocator allocator_{sizeof(Pair<MaterialId, Material>),
                                            256,
                                            memory::kEngineMemoryTagRendering};
  Map<MaterialId, Material*> materials_{};
  TextureHandler* texture_handler_{nullptr};
};
}  // namespace gl
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_OPENGL_HANDLER_OPENGL_MATERIAL_HANDLER_H_
