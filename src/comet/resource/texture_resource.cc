// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "texture_resource.h"

#include "comet/core/memory/memory_utils.h"
#include "comet/core/type/array.h"

namespace comet {
namespace resource {
const ResourceTypeId TextureResource::kResourceTypeId{
    COMET_STRING_ID("texture")};

ResourceId GetDefaultTextureFromType(rendering::TextureType texture_type) {
  switch (texture_type) {
    case rendering::TextureType::Diffuse:
      return kDefaultDiffuseTextureResourceId;
    case rendering::TextureType::Specular:
      return kDefaultSpecularTextureResourceId;
    case rendering::TextureType::Normal:
      return kDefaultNormalTextureResourceId;
    default:
      COMET_ASSERT(false, "Unknown or unsupported texture type provided: ",
                   static_cast<std::underlying_type_t<rendering::TextureType>>(
                       texture_type),
                   "!");
      return kDefaultResourceId;
  }
}

TextureHandler::TextureHandler(memory::Allocator* loading_resources_allocator,
                               memory::Allocator* loading_resource_allocator)
    : ResourceHandler{sizeof(TextureResource), loading_resources_allocator,
                      loading_resource_allocator} {}

void TextureHandler::Initialize() {
  ResourceHandler::Initialize();
  resource_data_allocator_.Initialize();
}

void TextureHandler::Shutdown() {
  resource_data_allocator_.Destroy();
  ResourceHandler::Shutdown();
}

void TextureHandler::Destroy(ResourceId resource_id) {
  COMET_ASSERT(resource_id != kDefaultDiffuseTextureResourceId,
               "Cannot unload default diffuse texture resource!");
  COMET_ASSERT(resource_id != kDefaultSpecularTextureResourceId,
               "Cannot unload default specular texture resource!");
  COMET_ASSERT(resource_id != kDefaultNormalTextureResourceId,
               "Cannot unload default normal texture resource!");
  return ResourceHandler::Destroy(resource_id);
}

Resource* TextureHandler::GetDefaultResource() {
  if (default_texture_ == nullptr) {
    constexpr auto kDimension{256};
    constexpr auto kPatternThreshold{kDimension / 16};
    constexpr auto kChannelCount{4};
    constexpr u8 kColor1[]{30, 30, 30, 255};
    constexpr u8 kColor2[]{200, 200, 200, 255};

    default_texture_ = std::make_unique<TextureResource>();
    default_texture_->id = kDefaultResourceId;
    default_texture_->type_id = TextureResource::kResourceTypeId;

    auto& descr{default_texture_->descr};
    descr.size =
        static_cast<comet::u64>(kDimension) * kDimension * kChannelCount;
    descr.format = rendering::TextureFormat::Rgba8;
    descr.resolution[0] = kDimension;
    descr.resolution[1] = kDimension;
    descr.channel_count = kChannelCount;

    default_texture_->data = Array<u8>{&resource_data_allocator_};
    auto& data{default_texture_->data};
    data.Resize(descr.size);
    auto is_color_1{false};

    for (usize col{0}; col < kDimension; ++col) {
      if (col % kPatternThreshold == 0) {
        is_color_1 = !is_color_1;
      }

      for (usize row{0}; row < kDimension; ++row) {
        const auto row_image_index{col * kDimension + row};
        const auto data_index{kChannelCount * row_image_index};

        if (row_image_index % kPatternThreshold == 0) {
          is_color_1 = !is_color_1;
        }

        const auto& color{is_color_1 ? kColor1 : kColor2};

        for (usize k{0}; k < kChannelCount; ++k) {
          data[data_index + k] = color[k];
        }
      }
    }
  }

  return default_texture_.get();
}

Resource* TextureHandler::GetDefaultDiffuseTexture() {
  if (diffuse_texture_ == nullptr) {
    constexpr auto kDimension{16};
    constexpr auto kChannelCount{4};
    constexpr u8 kColor[]{150, 150, 150, 255};

    diffuse_texture_ = std::make_unique<TextureResource>();
    diffuse_texture_->id = kDefaultDiffuseTextureResourceId;
    diffuse_texture_->type_id = TextureResource::kResourceTypeId;

    auto& descr{diffuse_texture_->descr};
    descr.size =
        static_cast<comet::u64>(kDimension) * kDimension * kChannelCount;
    descr.format = rendering::TextureFormat::Rgba8;
    descr.resolution[0] = kDimension;
    descr.resolution[1] = kDimension;
    descr.channel_count = kChannelCount;

    diffuse_texture_->data = Array<u8>{&resource_data_allocator_};
    auto& data{diffuse_texture_->data};
    data.Resize(descr.size);

    for (usize i{0}; i < descr.size; ++i) {
      data[i] = kColor[i % kChannelCount];
    }
  }

  return diffuse_texture_.get();
}

Resource* TextureHandler::GetDefaultSpecularTexture() {
  if (specular_texture_ == nullptr) {
    constexpr auto kDimension{1};
    constexpr auto kChannelCount{4};
    constexpr u8 kColor[]{0, 0, 0, 255};

    specular_texture_ = std::make_unique<TextureResource>();
    specular_texture_->id = kDefaultSpecularTextureResourceId;
    specular_texture_->type_id = TextureResource::kResourceTypeId;

    auto& descr{specular_texture_->descr};
    descr.size =
        static_cast<comet::u64>(kDimension) * kDimension * kChannelCount;
    descr.format = rendering::TextureFormat::Rgba8;
    descr.resolution[0] = kDimension;
    descr.resolution[1] = kDimension;
    descr.channel_count = kChannelCount;

    specular_texture_->data = Array<u8>{&resource_data_allocator_};
    auto& data{specular_texture_->data};
    data.Resize(descr.size);

    for (usize i{0}; i < descr.size; ++i) {
      data[i] = kColor[i % kChannelCount];
    }
  }

  return specular_texture_.get();
}

Resource* TextureHandler::GetDefaultNormalTexture() {
  if (normal_texture_ == nullptr) {
    constexpr auto kDimension{1};
    constexpr auto kChannelCount{4};
    constexpr u8 kColor[]{127, 127, 255, 255};

    normal_texture_ = std::make_unique<TextureResource>();
    normal_texture_->id = kDefaultNormalTextureResourceId;
    normal_texture_->type_id = TextureResource::kResourceTypeId;

    auto& descr{normal_texture_->descr};
    descr.size =
        static_cast<comet::u64>(kDimension) * kDimension * kChannelCount;
    descr.format = rendering::TextureFormat::Rgba8;
    descr.resolution[0] = kDimension;
    descr.resolution[1] = kDimension;
    descr.channel_count = kChannelCount;

    normal_texture_->data = Array<u8>{&resource_data_allocator_};
    auto& data{normal_texture_->data};
    data.Resize(descr.size);

    for (usize i{0}; i < descr.size; ++i) {
      data[i] = kColor[i % kChannelCount];
    }
  }

  return normal_texture_.get();
}

Resource* TextureHandler::GetInternal(ResourceId resource_id) {
  if (resource_id == kDefaultDiffuseTextureResourceId) {
    return GetDefaultDiffuseTexture();
  }

  if (resource_id == kDefaultSpecularTextureResourceId) {
    return GetDefaultSpecularTexture();
  }

  if (resource_id == kDefaultNormalTextureResourceId) {
    return GetDefaultNormalTexture();
  }

  return ResourceHandler::GetInternal(resource_id);
}

ResourceFile TextureHandler::Pack(memory::Allocator& allocator,
                                  const Resource& resource,
                                  CompressionMode compression_mode) const {
  const auto& texture{static_cast<const TextureResource&>(resource)};
  ResourceFile file{};
  file.resource_id = texture.id;
  file.resource_type_id = TextureResource::kResourceTypeId;
  file.compression_mode = compression_mode;
  file.descr = Array<u8>{&allocator};
  file.data = Array<u8>{&allocator};

  constexpr auto kResourceIdSize{sizeof(resource::ResourceId)};
  constexpr auto kResourceTypeIdSize{sizeof(resource::ResourceTypeId)};
  const auto data_size{sizeof(u8) * texture.data.GetSize()};

  Array<u8> data{&allocator};
  data.Resize(kResourceIdSize + kResourceTypeIdSize + data_size);
  usize cursor{0};
  auto* buffer{data.GetData()};

  memory::CopyMemory(&buffer[cursor], &texture.id, kResourceIdSize);
  cursor += kResourceIdSize;

  memory::CopyMemory(&buffer[cursor], &texture.type_id, kResourceTypeIdSize);
  cursor += kResourceTypeIdSize;

  memory::CopyMemory(&buffer[cursor], texture.data.GetData(), data_size);
  cursor += data_size;

  PackPodResourceDescr(texture.descr, file);
  PackResourceData(data, file);
  return file;
}

Resource* TextureHandler::Unpack(memory::Allocator& allocator,
                                 const ResourceFile& file) {
  auto* texture{resource_allocator_.AllocateOneAndPopulate<TextureResource>()};
  UnpackPodResourceDescr<TextureResourceDescr>(file, texture->descr);

  Array<u8> data{&allocator};
  UnpackResourceData(file, data);
  const auto* buffer{data.GetData()};
  usize cursor{0};

  constexpr auto kResourceIdSize{sizeof(resource::ResourceId)};
  constexpr auto kResourceTypeIdSize{sizeof(resource::ResourceTypeId)};
  const auto data_size{sizeof(u8) * data.GetSize() - kResourceIdSize -
                       kResourceTypeIdSize};

  memory::CopyMemory(&texture->id, &buffer[cursor], kResourceIdSize);
  cursor += kResourceIdSize;

  memory::CopyMemory(&texture->type_id, &buffer[cursor], kResourceTypeIdSize);
  cursor += kResourceTypeIdSize;

  texture->data = Array<u8>{&resource_data_allocator_};
  texture->data.Resize(data_size);
  memory::CopyMemory(texture->data.GetData(), &buffer[cursor], data_size);
  cursor += data_size;

  return texture;
}
}  // namespace resource
}  // namespace comet
