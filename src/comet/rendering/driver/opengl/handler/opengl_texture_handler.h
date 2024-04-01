// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_OPENGL_HANDLER_OPENGL_TEXTURE_HANDLER_H_
#define COMET_COMET_RENDERING_DRIVER_OPENGL_HANDLER_OPENGL_TEXTURE_HANDLER_H_

#include "glad/glad.h"

#include "comet/core/essentials.h"
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

  void Shutdown() override;

  const Texture* Generate(const resource::TextureResource* resource);
  const Texture* Get(TextureHandle texture_handle) const;
  const Texture* TryGet(TextureHandle texture_handle) const;
  const Texture* GetOrGenerate(const resource::TextureResource* resource);
  void Destroy(TextureHandle texture_handle);
  void Destroy(Texture& texture);

 private:
  Texture* Get(TextureHandle texture_handle);
  Texture* TryGet(TextureHandle texture_handle);
  void Destroy(Texture& texture, bool is_destroying_handler);
  static u32 GetMipLevels(const resource::TextureResource* resource);
  static GLenum GetGlFormat(const resource::TextureResource* resource);
  static GLenum GetGlInternalFormat(const resource::TextureResource* resource);

  void GenerateMipmaps(const Texture& texture) const;
  Texture GenerateInstance(const resource::TextureResource* resource) const;

  std::unordered_map<TextureHandle, Texture> textures_{};
};
}  // namespace gl
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_OPENGL_HANDLER_OPENGL_TEXTURE_HANDLER_H_
