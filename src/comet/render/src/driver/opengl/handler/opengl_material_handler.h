// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_RENDER_DRIVER_OPENGL_HANDLER_OPENGL_MATERIAL_HANDLER_H_
#define COMET_RENDER_DRIVER_OPENGL_HANDLER_OPENGL_MATERIAL_HANDLER_H_

#include "comet/core/essentials.h"
#include "comet/core/memory/allocator/free_list_allocator.h"
#include "comet/core/type/shared_instance_registry.h"
#include "comet/rendering/driver/opengl/handler/opengl_handler.h"
#include "comet/rendering/driver/opengl/handler/opengl_sampler_handler.h"
#include "comet/rendering/driver/opengl/handler/opengl_texture_handler.h"
#include "comet/rendering/driver/opengl/type/opengl_material.h"
#include "comet/rendering/driver/opengl/type/opengl_texture_map.h"
#include "comet/rendering/rendering_handle.h"
#include "comet/resource/material/material_resource.h"

namespace comet {
namespace rendering {
namespace gl {
using MaterialDestroyCallback = void (*)(const Material* material,
                                         void* user_data);

struct MaterialHandlerDescr : HandlerDescr {
  TextureHandler* texture_handler{nullptr};
  SamplerHandler* sampler_handler{nullptr};
};

class MaterialHandler : public Handler {
 public:
  MaterialHandler() = delete;
  explicit MaterialHandler(const MaterialHandlerDescr& descr);
  MaterialHandler(const MaterialHandler&) = delete;
  MaterialHandler(MaterialHandler&&) = delete;
  MaterialHandler& operator=(const MaterialHandler&) = delete;
  MaterialHandler& operator=(MaterialHandler&&) = delete;
  ~MaterialHandler() override = default;

  void SetDestroyCallback(MaterialDestroyCallback callback, void* user_data);

  MaterialHandle GetOrGenerate(const MaterialDescr& descr);
  MaterialHandle GetOrGenerate(
      resource::MaterialResourceId material_resource_id);

  void Destroy(MaterialHandle handle);

  const Material* Get(MaterialHandle handle) const;

 protected:
  void OnInitialize() override;
  void OnShutdown() override;

 private:
  TextureMap GenerateTextureMap(const resource::TextureMapResource* map);
  void DestroyMaterial(Material* material);

  SamplerHandle GetOrGenerateSampler(const resource::TextureMapResource* map);

  Material* Get(MaterialHandle handle);

  memory::PlatformAllocator cache_allocator_{memory::kEngineMemoryTagRendering};

  memory::FiberFreeListAllocator allocator_{sizeof(Material), 256,
                                            memory::kEngineMemoryTagRendering};

  SharedInstanceRegistry<resource::MaterialResourceId, MaterialTag, Material>
      materials_;

  MaterialDestroyCallback destroy_callback_{nullptr};
  void* destroy_callback_user_data_{nullptr};

  TextureHandler* texture_handler_{nullptr};
  SamplerHandler* sampler_handler_{nullptr};
};
}  // namespace gl
}  // namespace rendering
}  // namespace comet

#endif  // COMET_RENDER_DRIVER_OPENGL_HANDLER_OPENGL_MATERIAL_HANDLER_H_