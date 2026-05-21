// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_DATA_RESOURCE_RESOURCE_H_
#define COMET_DATA_RESOURCE_RESOURCE_H_

#include "comet/core/essentials.h"
#include "comet/core/hash.h"
#include "comet/core/string/tstring.h"
#include "comet/core/type/string_id.h"
#include "comet/data/resource/common.h"
#include "comet/data/resource/resource_id.h"

namespace comet {
namespace resource {
struct Resource {
  ResourceLifeSpan life_span{ResourceLifeSpan::Unknown};
  RawResourceId id{kInvalidRawResourceId};
  ResourceTypeId type_id{kInvalidResourceTypeId};

  virtual ~Resource() = default;
};

struct InternalResource {
  RawResourceId resource_id{kInvalidRawResourceId};
  RawResourceId internal_id{kInvalidRawResourceId};
};

template <typename ResourceType>
using ResourceIdOf = typename ResourceType::Id;

template <typename ResourceType>
ResourceIdOf<ResourceType> GenerateResourceIdFromPath(
    CTStringView resource_path) {
  return ResourceIdOf<ResourceType>{HashCombine(COMET_STRING_ID(resource_path),
                                                ResourceType::kResourceTypeId)};
}
}  // namespace resource
}  // namespace comet

#endif  // COMET_DATA_RESOURCE_RESOURCE_H_