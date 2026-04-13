// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet/rendering/comet_rendering_pch.h"
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "opengl_texture_handler.h"
////////////////////////////////////////////////////////////////////////////////

// External. ///////////////////////////////////////////////////////////////////
#include <type_traits>
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/memory/allocator/allocator.h"
#include "comet/math/math_common.h"
#include "comet/rendering/driver/opengl/opengl_debug.h"

namespace comet {
namespace rendering {
namespace gl {
TextureHandler::TextureHandler(const TextureHandlerDescr& descr)
    : Handler{descr} {}

void TextureHandler::Initialize() {
  Handler::Initialize();
  allocator_.Initialize();
  textures_ = Map<TextureKey, Texture*, TextureKeyHashLogic>{&allocator_};
}

void TextureHandler::Shutdown() {
  for (auto& it : textures_) {
    Destroy(it.value, true);
  }

  textures_.Destroy();
  allocator_.Destroy();
  Handler::Shutdown();
}

const Texture* TextureHandler::Generate(
    const resource::TextureResource* resource) {
  return Generate(resource, TextureType::Unknown);
}

const Texture* TextureHandler::Generate(
    const resource::TextureResource* resource, TextureType type) {
  COMET_ASSERT(resource != nullptr, "Texture resource is null!");

  TextureKey key{};
  key.resource_id = resource->id;
  key.type = type;

  auto* texture{textures_.Emplace(key, GenerateInstance(resource, type)).value};

  glGenTextures(1, &texture->handle);
  COMET_ASSERT(texture->handle != kInvalidTextureHandle,
               "Failed to generate OpenGL texture!");

  glBindTexture(GL_TEXTURE_2D, texture->handle);

#ifdef COMET_RENDERING_USE_DEBUG_LABELS
  COMET_GL_SET_TEXTURE_DEBUG_LABEL(texture->handle, "texture");
#endif  // COMET_RENDERING_USE_DEBUG_LABELS

  auto upload_format{GetGlFormat(resource)};

  glTexImage2D(GL_TEXTURE_2D, 0, texture->internal_format,
               static_cast<GLsizei>(texture->width),
               static_cast<GLsizei>(texture->height), 0, upload_format,
               GL_UNSIGNED_BYTE, resource->data.GetData());

  GenerateMipmaps(texture);

  glBindTexture(GL_TEXTURE_2D, kInvalidTextureHandle);
  return texture;
}

const Texture* TextureHandler::Get(TextureId texture_id) const {
  auto* texture{TryGet(texture_id)};
  COMET_ASSERT(texture != nullptr,
               "Requested texture does not exist: ", texture_id, "!");
  return texture;
}

const Texture* TextureHandler::Get(TextureId texture_id,
                                   TextureType type) const {
  auto* texture{TryGet(texture_id, type)};
  COMET_ASSERT(texture != nullptr,
               "Requested texture does not exist: ", texture_id, "!");
  return texture;
}

const Texture* TextureHandler::TryGet(TextureId texture_id) const {
  return TryGet(texture_id, TextureType::Unknown);
}

const Texture* TextureHandler::TryGet(TextureId texture_id,
                                      TextureType type) const {
  TextureKey key{};
  key.resource_id = texture_id;
  key.type = type;

  auto texture_ptr{textures_.TryGet(key)};

  if (texture_ptr == nullptr) {
    return nullptr;
  }

  auto* texture{*texture_ptr};
  ++texture->ref_count;
  return texture;
}

const Texture* TextureHandler::GetOrGenerate(
    const resource::TextureResource* resource) {
  return GetOrGenerate(resource, TextureType::Unknown);
}

const Texture* TextureHandler::GetOrGenerate(
    const resource::TextureResource* resource, TextureType type) {
  COMET_ASSERT(resource != nullptr, "Texture resource is null!");

  auto* texture{TryGet(resource->id, type)};

  if (texture != nullptr) {
    return texture;
  }

  return Generate(resource, type);
}

void TextureHandler::Destroy(TextureId texture_id) {
  Destroy(texture_id, TextureType::Unknown);
}

void TextureHandler::Destroy(TextureId texture_id, TextureType type) {
  Destroy(Get(texture_id, type));
}

void TextureHandler::Destroy(Texture* texture) { Destroy(texture, false); }

Texture* TextureHandler::Get(TextureId texture_id) {
  auto* texture{TryGet(texture_id)};
  COMET_ASSERT(texture != nullptr,
               "Requested texture does not exist: ", texture_id, "!");
  return texture;
}

Texture* TextureHandler::Get(TextureId texture_id, TextureType type) {
  auto* texture{TryGet(texture_id, type)};
  COMET_ASSERT(texture != nullptr,
               "Requested texture does not exist: ", texture_id, "!");
  return texture;
}

Texture* TextureHandler::TryGet(TextureId texture_id) {
  return TryGet(texture_id, TextureType::Unknown);
}

Texture* TextureHandler::TryGet(TextureId texture_id, TextureType type) {
  TextureKey key{};
  key.resource_id = texture_id;
  key.type = type;

  auto* texture_ptr{textures_.TryGet(key)};

  if (texture_ptr == nullptr) {
    return nullptr;
  }

  return *texture_ptr;
}

void TextureHandler::Destroy(Texture* texture, bool is_destroying_handler) {
  COMET_ASSERT(texture != nullptr, "Texture is null!");

  if (!is_destroying_handler) {
    COMET_ASSERT(texture->ref_count > 0, "Texture has a reference count of 0!");

    if (--texture->ref_count > 0) {
      return;
    }
  }

  if (texture->handle != kInvalidTextureHandle) {
    glDeleteTextures(1, &texture->handle);
    texture->handle = kInvalidTextureHandle;
  }

  if (!is_destroying_handler) {
    TextureKey key{};
    key.resource_id = texture->id;
    key.type = texture->type;
    textures_.Remove(key);
  }

  allocator_.Deallocate(texture);
}

u32 TextureHandler::GetMipLevels(const resource::TextureResource* resource) {
  return static_cast<u32>(math::Log2(math::Max(
             resource->descr.resolution[0], resource->descr.resolution[1]))) +
         1;
}

bool TextureHandler::IsSrgbTextureType(TextureType type) {
  switch (type) {
    case TextureType::Diffuse:
    case TextureType::Color:
      return true;

    case TextureType::Specular:
    case TextureType::Normal:
    case TextureType::Ambient:
    case TextureType::Unknown:
    default:
      return false;
  }
}

GLenum TextureHandler::GetGlFormat(const resource::TextureResource* resource) {
  switch (resource->descr.format) {
    case rendering::TextureFormat::Rgba8:
      return GL_RGBA;

    case rendering::TextureFormat::Rgb8:
      return GL_RGB;

    case rendering::TextureFormat::Unknown:
    default:
      COMET_ASSERT(
          false, "Unknown or unsupported texture format: ",
          static_cast<std::underlying_type_t<rendering::TextureFormat>>(
              resource->descr.format),
          "!");
      return GL_RGBA;
  }
}

GLenum TextureHandler::GetGlInternalFormat(
    const resource::TextureResource* resource, TextureType type) {
  auto is_srgb{IsSrgbTextureType(type)};

  switch (resource->descr.format) {
    case rendering::TextureFormat::Rgba8:
      return is_srgb ? GL_SRGB8_ALPHA8 : GL_RGBA8;

    case rendering::TextureFormat::Rgb8:
      return is_srgb ? GL_SRGB8_ALPHA8 : GL_RGBA8;

    case rendering::TextureFormat::Unknown:
    default:
      COMET_ASSERT(
          false, "Unknown or unsupported texture format: ",
          static_cast<std::underlying_type_t<rendering::TextureFormat>>(
              resource->descr.format),
          "!");
      return GL_RGBA8;
  }
}

u8 TextureHandler::GetResolvedChannelCount(
    const resource::TextureResource* resource) {
  switch (resource->descr.format) {
    case rendering::TextureFormat::Rgba8:
      return 4;

    case rendering::TextureFormat::Rgb8:
      return 4;

    default:
      return resource->descr.channel_count;
  }
}

void TextureHandler::GenerateMipmaps(const Texture* texture) const {
  COMET_ASSERT(texture != nullptr, "Texture is null!");
  COMET_ASSERT(texture->handle != kInvalidTextureHandle,
               "Texture handle is invalid!");

  glBindTexture(GL_TEXTURE_2D, texture->handle);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL,
                  static_cast<GLint>(texture->mip_levels - 1));
  glGenerateMipmap(GL_TEXTURE_2D);
}

Texture* TextureHandler::GenerateInstance(
    const resource::TextureResource* resource, TextureType type) {
  auto* texture{allocator_.AllocateOneAndPopulate<Texture>()};
  texture->id = resource->id;
  texture->type = type;
  texture->ref_count = 1;
  texture->width = resource->descr.resolution[0];
  texture->height = resource->descr.resolution[1];
  texture->depth = resource->descr.resolution[2];
  texture->mip_levels = GetMipLevels(resource);
  texture->channel_count = GetResolvedChannelCount(resource);
  texture->format = GetGlFormat(resource);
  texture->internal_format = GetGlInternalFormat(resource, type);
  return texture;
}
}  // namespace gl
}  // namespace rendering
}  // namespace comet