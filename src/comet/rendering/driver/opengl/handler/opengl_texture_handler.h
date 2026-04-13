// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_OPENGL_HANDLER_OPENGL_TEXTURE_HANDLER_H_
#define COMET_COMET_RENDERING_DRIVER_OPENGL_HANDLER_OPENGL_TEXTURE_HANDLER_H_

// External. ///////////////////////////////////////////////////////////////////
#include "glad/glad.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/essentials.h"
#include "comet/core/memory/allocator/free_list_allocator.h"
#include "comet/core/type/map.h"
#include "comet/rendering/driver/opengl/data/opengl_texture.h"
#include "comet/rendering/driver/opengl/handler/opengl_handler.h"
#include "comet/resource/texture_resource.h"

namespace comet {
namespace rendering {
namespace gl {
using TextureHandlerDescr = HandlerDescr;

class TextureHandler : public Handler {
 public:
  TextureHandler() = delete;
  explicit TextureHandler(const TextureHandlerDescr& descr);
  TextureHandler(const TextureHandler&) = delete;
  TextureHandler(TextureHandler&&) = delete;
  TextureHandler& operator=(const TextureHandler&) = delete;
  TextureHandler& operator=(TextureHandler&&) = delete;
  virtual ~TextureHandler() = default;

  void Initialize() override;
  void Shutdown() override;

  const Texture* Generate(const resource::TextureResource* resource);
  const Texture* Generate(const resource::TextureResource* resource,
                          TextureType type);

  const Texture* Get(TextureId texture_id) const;
  const Texture* Get(TextureId texture_id, TextureType type) const;

  const Texture* TryGet(TextureId texture_id) const;
  const Texture* TryGet(TextureId texture_id, TextureType type) const;

  const Texture* GetOrGenerate(const resource::TextureResource* resource);
  const Texture* GetOrGenerate(const resource::TextureResource* resource,
                               TextureType type);

  void Destroy(TextureId texture_id);
  void Destroy(TextureId texture_id, TextureType type);
  void Destroy(Texture* texture);

 private:
  Texture* Get(TextureId texture_id);
  Texture* Get(TextureId texture_id, TextureType type);

  Texture* TryGet(TextureId texture_id);
  Texture* TryGet(TextureId texture_id, TextureType type);

  void Destroy(Texture* texture, bool is_destroying_handler);

  static u32 GetMipLevels(const resource::TextureResource* resource);
  static bool IsSrgbTextureType(TextureType type);
  static GLenum GetGlFormat(const resource::TextureResource* resource);
  static GLenum GetGlInternalFormat(const resource::TextureResource* resource,
                                    TextureType type);
  static u8 GetResolvedChannelCount(const resource::TextureResource* resource);

  void GenerateMipmaps(const Texture* texture) const;
  Texture* GenerateInstance(const resource::TextureResource* resource,
                            TextureType type);

  memory::FiberFreeListAllocator allocator_{sizeof(Texture), 256,
                                            memory::kEngineMemoryTagRendering};
  Map<TextureKey, Texture*, TextureKeyHashLogic> textures_{};
};
}  // namespace gl
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_OPENGL_HANDLER_OPENGL_TEXTURE_HANDLER_H_