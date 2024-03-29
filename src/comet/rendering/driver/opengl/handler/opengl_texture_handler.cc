// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "opengl_texture_handler.h"

#include "comet/math/math_commons.h"

namespace comet {
namespace rendering {
namespace gl {
TextureHandler::TextureHandler(const TextureHandlerDescr& descr)
    : Handler{descr} {}

void TextureHandler::Shutdown() {
  std::vector<TextureHandle> handles{};
  handles.reserve(textures_.size());

  for (auto& it : textures_) {
    auto& texture{it.second};

    if (texture.handle != kInvalidTextureHandle) {
      handles.push_back(texture.handle);
    }

    Destroy(it.second, true);
  }

  if (textures_.size() > 0) {
    glDeleteTextures(static_cast<s32>(handles.size()), handles.data());
  }

  textures_.clear();
  Handler::Shutdown();
}

const Texture* TextureHandler::Generate(
    const resource::TextureResource* resource) {
  Texture texture{};

  glGenTextures(1, &texture.handle);
  texture.width = resource->descr.resolution[0];
  texture.height = resource->descr.resolution[1];
  texture.depth = resource->descr.resolution[2];
  texture.mip_levels = GetMipLevels(resource);
  texture.format = GetGlFormat(resource);
  texture.internal_format = GetGlInternalFormat(resource);
  texture.channel_count = resource->descr.channel_count;

  glBindTexture(GL_TEXTURE_2D, texture.handle);
  glTexImage2D(GL_TEXTURE_2D, 0, texture.internal_format,
               resource->descr.resolution[0], resource->descr.resolution[1], 0,
               texture.format, GL_UNSIGNED_BYTE, resource->data.data());

  glGenerateMipmap(GL_TEXTURE_2D);

#ifdef COMET_DEBUG
  const auto texture_handle{texture.handle};
#endif  // COMET_DEBUG
  auto insert_pair{textures_.emplace(texture.handle, std::move(texture))};
  COMET_ASSERT(insert_pair.second, "Could not insert texture: ",
               COMET_STRING_ID_LABEL(texture_handle), "!");
  return &insert_pair.first->second;
}

const Texture* TextureHandler::Get(TextureHandle texture_handle) const {
  const auto* texture{TryGet(texture_handle)};
  COMET_ASSERT(texture != nullptr,
               "Requested texture does not exist: ", texture_handle, "!");
  return texture;
}

const Texture* TextureHandler::TryGet(TextureHandle texture_handle) const {
  auto it{textures_.find(texture_handle)};

  if (it == textures_.end()) {
    return nullptr;
  }

  return &it->second;
}

const Texture* TextureHandler::GetOrGenerate(
    const resource::TextureResource* resource) {
  const auto* texture{TryGet(resource->id)};

  if (texture != nullptr) {
    return texture;
  }

  return Generate(resource);
}

void TextureHandler::Destroy(TextureHandle texture_handle) {
  return Destroy(*Get(texture_handle));
}

void TextureHandler::Destroy(Texture& texture) {
  return Destroy(texture, false);
}

Texture* TextureHandler::Get(TextureHandle texture_handle) {
  auto* texture{TryGet(texture_handle)};
  COMET_ASSERT(texture != nullptr,
               "Requested texture does not exist: ", texture_handle, "!");
  return texture;
}

Texture* TextureHandler::TryGet(TextureHandle texture_handle) {
  auto it{textures_.find(texture_handle)};

  if (it == textures_.end()) {
    return nullptr;
  }

  return &it->second;
}

void TextureHandler::Destroy(Texture& texture, bool is_destroying_handler) {
  if (!is_destroying_handler) {
    if (texture.handle != kInvalidTextureHandle) {
      glDeleteTextures(1, &texture.handle);
    }

    textures_.erase(texture.handle);
  }

  texture.handle = kInvalidTextureHandle;
  texture.format = GL_INVALID_VALUE;
  texture.internal_format = GL_INVALID_VALUE;
  texture.width = 0;
  texture.height = 0;
  texture.depth = 0;
  texture.mip_levels = 0;
  texture.channel_count = 0;
}

u32 TextureHandler::GetMipLevels(const resource::TextureResource* resource) {
  return static_cast<u32>(math::Log2(math::Max(
             resource->descr.resolution[0], resource->descr.resolution[1]))) +
         1;
}

GLenum TextureHandler::GetGlFormat(const resource::TextureResource* resource) {
  GLenum format;

  switch (resource->descr.format) {
    case rendering::TextureFormat::Rgba8:
      format = GL_RGBA;
      break;

    case rendering::TextureFormat::Rgb8:
      format = GL_RGB;
      break;

    case rendering::TextureFormat::Unknown:
      format = GL_RGB8;
      break;  // Default behavior.

    default:
      format = GL_RGB8;

      COMET_ASSERT(
          false, "Unknown or unsupported texture format: ",
          static_cast<std::underlying_type_t<rendering::TextureFormat>>(
              resource->descr.format),
          "!");
  }

  return format;
}

GLenum TextureHandler::GetGlInternalFormat(
    const resource::TextureResource* resource) {
  GLenum internal_format;

  switch (resource->descr.format) {
    case rendering::TextureFormat::Rgba8:
      internal_format = GL_RGBA8;
      break;

    case rendering::TextureFormat::Rgb8:
      internal_format = GL_RGB8;
      break;

    case rendering::TextureFormat::Unknown:
      internal_format = GL_RGB8;
      break;  // Default behavior.

    default:
      internal_format = GL_RGB8;

      COMET_ASSERT(
          false, "Unknown or unsupported texture format: ",
          static_cast<std::underlying_type_t<rendering::TextureFormat>>(
              resource->descr.format),
          "!");
  }

  return internal_format;
}
}  // namespace gl
}  // namespace rendering
}  // namespace comet
