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

#include "comet/core/memory/allocator/allocator.h"
#include "comet/core/type_trait.h"
#include "comet/rendering/driver/opengl/label/opengl_texture_label.h"
#include "comet/rendering/driver/opengl/opengl_debug.h"
#include "comet/rendering/driver/opengl/utils/opengl_texture_utils.h"
#include "comet/rendering/label/rendering_texture_label.h"
#include "comet/rendering/utils/rendering_texture_utils.h"
#include "comet/resource/resource_manager.h"

namespace comet {
namespace rendering {
namespace gl {
TextureHandler::TextureHandler(const TextureHandlerDescr& descr)
    : Handler{descr}, textures_{&cache_allocator_, 1024} {}

TextureHandle TextureHandler::GetOrGenerate(
    resource::TextureResourceId texture_resource_id) {
  return GetOrGenerate(texture_resource_id, TextureType::Unknown);
}

TextureHandle TextureHandler::GetOrGenerate(
    resource::TextureResourceId texture_resource_id, TextureType type) {
  COMET_ASSERT(texture_resource_id.IsValid(), "TextureHandler::GetOrGenerate",
               "texture resource id is invalid");

  TextureKey key{
      .kind = TextureKeyKind::Resource,
      .texture_resource_id = texture_resource_id,
      .runtime_id = kInvalidRuntimeTextureId,
      .type = type,
  };

  if (const auto handle{textures_.TryAcquire(key)}; handle) {
    return handle;
  }

  TextureHandle generated_handle{TextureHandle::Invalid()};
  auto* texture_resource_handler{
      resource::ResourceManager::Get().GetTextures()};

  const auto is_loaded{texture_resource_handler->WithTemporaryLoad(
      texture_resource_id,
      [this, &generated_handle, type,
       texture_resource_id](const resource::TextureResource* texture_resource) {
        auto* texture{GenerateTexture(texture_resource, type)};
        COMET_ASSERT(texture != nullptr, "TextureHandler::GetOrGenerate",
                     "generated texture is null", "texture_resource_id",
                     texture_resource_id);

        TextureKey key{
            .kind = TextureKeyKind::Resource,
            .texture_resource_id = texture->texture_resource_id,
            .runtime_id = texture->runtime_id,
            .type = texture->type,
        };

        generated_handle = textures_.Create(key, texture);
        COMET_ASSERT(generated_handle, "TextureHandler::GetOrGenerate",
                     "texture instance creation failed", "texture_resource_id",
                     texture_resource_id, "texture_type",
                     GetTextureTypeLabel(type), "texture_type_value",
                     ToUnderlying(type));

        texture->handle = generated_handle;
      })};

  return is_loaded ? generated_handle : TextureHandle::Invalid();
}

TextureHandle TextureHandler::Generate(const RuntimeTextureDescr& descr) {
  COMET_ASSERT(descr.width > 0, "TextureHandler::Generate",
               "runtime texture width is zero");
  COMET_ASSERT(descr.height > 0, "TextureHandler::Generate",
               "runtime texture height is zero");
  COMET_ASSERT(descr.format != GL_INVALID_VALUE, "TextureHandler::Generate",
               "runtime texture format is invalid");
  COMET_ASSERT(descr.internal_format != GL_INVALID_VALUE,
               "TextureHandler::Generate",
               "runtime texture internal format is invalid");
  COMET_ASSERT(
      descr.target == GL_TEXTURE_2D || descr.target == GL_TEXTURE_2D_ARRAY,
      "TextureHandler::Generate", "runtime texture target is unsupported",
      "target", descr.target);

  if (descr.target == GL_TEXTURE_2D_ARRAY) {
    COMET_ASSERT(descr.depth > 0, "TextureHandler::Generate",
                 "runtime texture array depth is zero");
  }

  auto* texture{allocator_.AllocateOneAndPopulate<Texture>()};
  texture->handle = TextureHandle::Invalid();

  COMET_ASSERT(next_runtime_texture_id_ != kInvalidRuntimeTextureId,
               "TextureHandler::Generate", "runtime texture id overflow");

  texture->is_runtime = true;
  texture->texture_resource_id = resource::TextureResourceId::Invalid();
  texture->runtime_id = next_runtime_texture_id_++;

  texture->type = descr.type;
  texture->target = descr.target;
  texture->width = descr.width;
  texture->height = descr.height;
  texture->depth = descr.depth;
  texture->mip_levels = descr.mip_levels;
  texture->channel_count = descr.channel_count;
  texture->format = descr.format;
  texture->internal_format = descr.internal_format;

  glGenTextures(1, &texture->native_handle);
  COMET_ASSERT(texture->native_handle != kInvalidGlNativeTextureHandle,
               "TextureHandler::Generate",
               "opengl runtime texture creation failed", "runtime_texture_id",
               texture->runtime_id);

  glBindTexture(texture->target, texture->native_handle);

#ifdef COMET_RENDERING_USE_DEBUG_LABELS
  COMET_GL_SET_TEXTURE_DEBUG_LABEL(texture->native_handle, "runtime_texture");
#endif  // COMET_RENDERING_USE_DEBUG_LABELS

  switch (texture->target) {
    case GL_TEXTURE_2D:
      glTexImage2D(texture->target, 0,
                   static_cast<GLint>(texture->internal_format),
                   static_cast<GLsizei>(texture->width),
                   static_cast<GLsizei>(texture->height), 0, texture->format,
                   GL_UNSIGNED_BYTE, nullptr);
      break;

    case GL_TEXTURE_2D_ARRAY:
      glTexImage3D(texture->target, 0,
                   static_cast<GLint>(texture->internal_format),
                   static_cast<GLsizei>(texture->width),
                   static_cast<GLsizei>(texture->height),
                   static_cast<GLsizei>(texture->depth), 0, texture->format,
                   GL_UNSIGNED_BYTE, nullptr);
      break;

    default:
      COMET_ASSERT(false, "TextureHandler::Generate",
                   "runtime texture target is unsupported", "target",
                   texture->target);
      glBindTexture(texture->target, kInvalidGlNativeTextureHandle);
      glDeleteTextures(1, &texture->native_handle);
      texture->native_handle = kInvalidGlNativeTextureHandle;
      allocator_.Deallocate(texture);
      return TextureHandle::Invalid();
  }

  glBindTexture(texture->target, kInvalidGlNativeTextureHandle);

  TextureKey key{
      .kind = TextureKeyKind::Runtime,
      .texture_resource_id = texture->texture_resource_id,
      .runtime_id = texture->runtime_id,
      .type = texture->type,
  };

  const auto handle{textures_.Create(key, texture)};
  COMET_ASSERT(handle, "TextureHandler::Generate",
               "texture instance creation failed", "runtime_texture_id",
               texture->runtime_id, "texture_type",
               GetTextureTypeLabel(texture->type), "texture_type_value",
               ToUnderlying(texture->type));

  texture->handle = handle;
  return handle;
}

void TextureHandler::Destroy(TextureHandle handle) {
  auto* texture{textures_.Get(handle)};

  if (!textures_.Release(handle)) {
    return;
  }

  DestroyTexture(texture);
  textures_.Remove(handle);
}

const Texture* TextureHandler::Get(TextureHandle handle) const {
  const auto* texture{textures_.TryGet(handle)};
  COMET_ASSERT(texture != nullptr, "TextureHandler::Get", "texture not found",
               "texture_handle", handle);
  return texture;
}

void TextureHandler::OnInitialize() {
  allocator_.Initialize();
  textures_.Initialize();
}

void TextureHandler::OnShutdown() {
  memory::PlatformAllocator tmp_allocator{memory::kEngineMemoryTagRendering};
  Array<TextureHandle> handles_to_destroy{&tmp_allocator};

  textures_.ForEachLive(
      [&handles_to_destroy](TextureHandle handle, const Texture*) {
        handles_to_destroy.PushBack(handle);
      });

  for (const auto handle : handles_to_destroy) {
    const auto ref_count{textures_.GetRefCount(handle)};

    if (ref_count > 0) {
      const auto* texture{textures_.Get(handle)};
      [[maybe_unused]] const auto key{
          texture->is_runtime
              ? TextureKey{.kind = TextureKeyKind::Runtime,
                           .texture_resource_id = texture->texture_resource_id,
                           .runtime_id = texture->runtime_id,
                           .type = texture->type}
              : TextureKey{.kind = TextureKeyKind::Resource,
                           .texture_resource_id = texture->texture_resource_id,
                           .runtime_id = texture->runtime_id,
                           .type = texture->type}};

      COMET_LOG_WARNING(LoggerType::Rendering, "TextureHandler::OnShutdown",
                        "forcing texture destruction", "texture_handle", handle,
                        "ref_count", ref_count, "key_kind",
                        GetTextureKeyKindLabel(key.kind), "key_kind_value",
                        ToUnderlying(key.kind), "texture_resource_id",
                        key.texture_resource_id, "runtime_id", key.runtime_id,
                        "texture_type", GetTextureTypeLabel(key.type),
                        "texture_type_value", ToUnderlying(key.type));
    }

    auto* texture{textures_.Drain(handle)};

    if (texture == nullptr) {
      continue;
    }

    COMET_ASSERT(texture->handle == handle, "TextureHandler::OnShutdown",
                 "texture handle mismatch", "expected_handle", handle,
                 "actual_handle", texture->handle, "texture_resource_id",
                 texture->texture_resource_id, "runtime_id",
                 texture->runtime_id);

    DestroyTexture(texture);
  }

  textures_.Destroy();
  allocator_.Destroy();
}

Texture* TextureHandler::Get(TextureHandle handle) {
  auto* texture{textures_.TryGet(handle)};
  COMET_ASSERT(texture != nullptr, "TextureHandler::Get", "texture is null");
  return texture;
}

Texture* TextureHandler::GenerateTexture(
    const resource::TextureResource* resource, TextureType type) {
  COMET_ASSERT(resource != nullptr, "TextureHandler::GenerateTexture",
               "texture resource is null");

  auto* texture{allocator_.AllocateOneAndPopulate<Texture>()};
  texture->handle = TextureHandle::Invalid();

  texture->is_runtime = false;
  texture->texture_resource_id = resource->GetId();
  texture->runtime_id = kInvalidRuntimeTextureId;

  const auto& resource_descr{resource->descr};

  texture->type = type;
  texture->target = GL_TEXTURE_2D;
  texture->width = resource_descr.resolution[0];
  texture->height = resource_descr.resolution[1];
  texture->depth = resource_descr.resolution[2];
  texture->mip_levels = GetMipLevels(texture->width, texture->height);
  texture->channel_count = GetResolvedChannelCount(
      resource_descr.format, static_cast<u8>(resource_descr.channel_count));
  texture->format = GetGlFormat(resource);
  texture->internal_format = GetGlInternalFormat(resource, type);

  glGenTextures(1, &texture->native_handle);
  COMET_ASSERT(texture->native_handle != kInvalidGlNativeTextureHandle,
               "TextureHandler::GenerateTexture",
               "opengl texture creation failed", "texture_resource_id",
               texture->texture_resource_id);

  glBindTexture(texture->target, texture->native_handle);

#ifdef COMET_RENDERING_USE_DEBUG_LABELS
  COMET_GL_SET_TEXTURE_DEBUG_LABEL(texture->native_handle, "texture");
#endif  // COMET_RENDERING_USE_DEBUG_LABELS

  glTexImage2D(texture->target, 0, static_cast<GLint>(texture->internal_format),
               static_cast<GLsizei>(texture->width),
               static_cast<GLsizei>(texture->height), 0, texture->format,
               GL_UNSIGNED_BYTE, resource->data.GetData());

  GenerateMipmaps(texture);
  glBindTexture(texture->target, kInvalidGlNativeTextureHandle);
  return texture;
}

void TextureHandler::DestroyTexture(Texture* texture) {
  COMET_ASSERT(texture != nullptr, "TextureHandler::DestroyTexture",
               "texture is null");

  if (texture->native_handle != kInvalidGlNativeTextureHandle) {
    glDeleteTextures(1, &texture->native_handle);
    texture->native_handle = kInvalidGlNativeTextureHandle;
  }

  texture->handle.Invalidate();
  allocator_.Deallocate(texture);
}

void TextureHandler::GenerateMipmaps(const Texture* texture) const {
  COMET_ASSERT(texture != nullptr, "TextureHandler::GenerateMipmaps",
               "texture is null");
  COMET_ASSERT(texture->native_handle != kInvalidGlNativeTextureHandle,
               "TextureHandler::GenerateMipmaps",
               "texture native handle is invalid", "texture_handle",
               texture->handle, "texture_resource_id",
               texture->texture_resource_id, "runtime_id", texture->runtime_id);

  glBindTexture(texture->target, texture->native_handle);
  glTexParameteri(texture->target, GL_TEXTURE_BASE_LEVEL, 0);
  glTexParameteri(texture->target, GL_TEXTURE_MAX_LEVEL,
                  static_cast<GLint>(texture->mip_levels - 1));
  glGenerateMipmap(texture->target);
  glBindTexture(texture->target, kInvalidGlNativeTextureHandle);
}
}  // namespace gl
}  // namespace rendering
}  // namespace comet