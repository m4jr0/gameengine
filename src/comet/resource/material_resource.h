// Copyright 2022 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RESOURCE_MATERIAL_RESOURCE_H_
#define COMET_COMET_RESOURCE_MATERIAL_RESOURCE_H_

#include "comet_precompile.h"

#include "comet/rendering/rendering_common.h"
#include "comet/resource/resource.h"

namespace comet {
namespace resource {
using MaterialId = stringid::StringId;
constexpr auto kInvalidMaterialId{static_cast<stringid::StringId>(-1)};

struct MaterialResourceDescr {};

struct TextureTuple {
  ResourceId texture_id{kInvalidResourceId};
  rendering::TextureType type{rendering::TextureType::Unknown};
};

struct MaterialResource : public Resource {
  static const ResourceTypeId kResourceTypeId;

  std::vector<TextureTuple> texture_tuples;

  MaterialResourceDescr descr;
};

resource::MaterialId GenerateMaterialId(const std::string& material_name);
resource::MaterialId GenerateMaterialId(const char* material_name);

class MaterialHandler : public ResourceHandler {
 public:
  MaterialHandler() = default;
  MaterialHandler(const MaterialHandler&) = delete;
  MaterialHandler(MaterialHandler&&) = delete;
  MaterialHandler& operator=(const MaterialHandler&) = delete;
  MaterialHandler& operator=(MaterialHandler&&) = delete;
  ~MaterialHandler() = default;

 protected:
  ResourceFile Pack(const Resource& resource,
                    CompressionMode compression_mode) const override;
  std::unique_ptr<Resource> Unpack(const ResourceFile& file) const override;
};
}  // namespace resource
}  // namespace comet

#endif  // COMET_COMET_RESOURCE_MATERIAL_RESOURCE_H_
