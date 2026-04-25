// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RESOURCE_TYPE_COMMON_H_
#define COMET_COMET_RESOURCE_TYPE_COMMON_H_

// External. ///////////////////////////////////////////////////////////////////
#include <string_view>
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/essentials.h"
#include "comet/core/type/string_id.h"

namespace comet {
namespace resource {
using RawResourceId = stringid::StringId;
constexpr auto kInvalidRawResourceId{static_cast<RawResourceId>(-1)};
constexpr auto kFallbackRawResourceId{0};

using ResourceTypeId = stringid::StringId;
constexpr auto kInvalidResourceTypeId{static_cast<ResourceTypeId>(-1)};

using ResourceTypeName = std::string_view;

enum class CompressionMode : u8 { None = 0, Lz4 };

enum class ResourceLifeSpan : u8 {
  Unknown = 0,
  Manual,
  Scene,
  Global,
  Immortal
};
}  // namespace resource
}  // namespace comet

#endif  // COMET_COMET_RESOURCE_TYPE_COMMON_H_