// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_OPENGL_HANDLER_OPENGL_MATERIAL_HANDLER_H_
#define COMET_COMET_RENDERING_DRIVER_OPENGL_HANDLER_OPENGL_MATERIAL_HANDLER_H_

#include "comet/core/essentials.h"
#include "comet/core/memory/allocator/free_list_allocator.h"
#include "comet/core/type/map.h"
#include "comet/math/math_common.h"
#include "comet/rendering/driver/opengl/data/opengl_material.h"
#include "comet/rendering/driver/opengl/handler/opengl_handler.h"
#include "comet/rendering/driver/opengl/handler/opengl_texture_handler.h"
#include "comet/resource/material_resource.h"
#include "comet/resource/resource.h"

namespace comet {
namespace rendering {
namespace gl {
using MaterialDestroyCallback = void (*)(Material* material, void* user_data);

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

  void SetDestroyCallback(MaterialDestroyCallback callback, void* user_data);

  Material* Generate(const MaterialDescr& descr);
  Material* Generate(const resource::MaterialResource* resource);
  Material* Get(MaterialId material_id);
  Material* TryGet(MaterialId material_id);
  void Destroy(MaterialId material_id);
  void Destroy(Material* material);

 private:
  TextureMap GenerateTextureMap(const resource::TextureMap* map,
                                resource::ResourceLifeSpan life_span =
                                    resource::ResourceLifeSpan::Manual);
  void Destroy(Material* material, bool is_destroying_handler);

  Sampler* GenerateSampler(SamplerId sampler_id, GLenum wrap_s, GLenum wrap_t,
                           GLenum min_filter, GLenum mag_filter);
  Sampler* GetSampler(SamplerId sampler_id);
  Sampler* TryGetSampler(SamplerId sampler_id);
  Sampler* GetOrGenerateSampler(const resource::TextureMap* texture_map);
  void Destroy(Sampler* sampler);

  static GLenum GetWrapMode(TextureRepeatMode repeat_mode);
  static GLenum GetFilterMode(TextureFilterMode filter_mode);

  memory::FiberFreeListAllocator allocator_{
      math::Max(sizeof(Pair<MaterialId, Material>),
                sizeof(Pair<SamplerId, Sampler>)),
      256, memory::kEngineMemoryTagRendering};

  MaterialDestroyCallback destroy_callback_{nullptr};
  void* destroy_callback_user_data_{nullptr};

  Map<MaterialId, Material*> materials_{};
  Map<SamplerId, Sampler*> samplers_{};

  TextureHandler* texture_handler_{nullptr};
};
}  // namespace gl
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_OPENGL_HANDLER_OPENGL_MATERIAL_HANDLER_H_