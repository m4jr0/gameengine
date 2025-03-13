// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "comet/rendering/comet_rendering_pch.h"
#include "comet_pch.h"

#include "opengl_texture_handler.h"

#include <utility>

#include "comet/core/frame/frame_utils.h"
#include "comet/math/math_commons.h"

namespace comet {
namespace rendering {
namespace gl {
TextureHandler::TextureHandler(const TextureHandlerDescr& descr)
    : Handler{descr} {}

void TextureHandler::Initialize() {
  Handler::Initialize();
  allocator_.Initialize();
  textures_ = Map<TextureHandle, Texture*>{&allocator_};
}

void TextureHandler::Shutdown() {
  frame::FrameArray<TextureHandle> handles{};
  handles.Reserve(textures_.GetEntryCount());

  for (auto& it : textures_) {
    auto& texture{it.value};

    if (texture->handle != kInvalidTextureHandle) {
      handles.PushBack(texture->handle);
    }

    Destroy(it.value, true);
  }

  if (!handles.IsEmpty()) {
    glDeleteTextures(static_cast<s32>(handles.GetSize()), handles.GetData());
  }

  textures_.Clear();
  allocator_.Destroy();
  Handler::Shutdown();
}

const Texture* TextureHandler::Generate(
    const resource::TextureResource* resource) {
  auto* texture{allocator_.AllocateOneAndPopulate<Texture>()};

  glGenTextures(1, &texture->handle);
  texture->width = resource->descr.resolution[0];
  texture->height = resource->descr.resolution[1];
  texture->depth = resource->descr.resolution[2];
  texture->mip_levels = GetMipLevels(resource);
  texture->format = GetGlFormat(resource);
  texture->internal_format = GetGlInternalFormat(resource);
  texture->channel_count = resource->descr.channel_count;

  glBindTexture(GL_TEXTURE_2D, texture->handle);
  glTexImage2D(GL_TEXTURE_2D, 0, texture->internal_format,
               resource->descr.resolution[0], resource->descr.resolution[1], 0,
               texture->format, GL_UNSIGNED_BYTE, resource->data.GetData());

  glGenerateMipmap(GL_TEXTURE_2D);
  return textures_.Emplace(texture->handle, texture).value;
}

const Texture* TextureHandler::Get(TextureHandle texture_handle) const {
  const auto* texture{TryGet(texture_handle)};
  COMET_ASSERT(texture != nullptr,
               "Requested texture does not exist: ", texture_handle, "!");
  return texture;
}

const Texture* TextureHandler::TryGet(TextureHandle texture_handle) const {
  auto texture{textures_.TryGet(texture_handle)};

  if (texture == nullptr) {
    return nullptr;
  }

  return *texture;
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
  return Destroy(Get(texture_handle));
}

void TextureHandler::Destroy(Texture* texture) {
  return Destroy(texture, false);
}

Texture* TextureHandler::Get(TextureHandle texture_handle) {
  auto* texture{TryGet(texture_handle)};
  COMET_ASSERT(texture != nullptr,
               "Requested texture does not exist: ", texture_handle, "!");
  return texture;
}

Texture* TextureHandler::TryGet(TextureHandle texture_handle) {
  auto texture{textures_.TryGet(texture_handle)};

  if (texture == nullptr) {
    return nullptr;
  }

  return *texture;
}

void TextureHandler::Destroy(Texture* texture, bool is_destroying_handler) {
  if (!is_destroying_handler) {
    if (texture->handle != kInvalidTextureHandle) {
      glDeleteTextures(1, &texture->handle);
    }

    textures_.Remove(texture->handle);
  }

  allocator_.Deallocate(texture);
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
