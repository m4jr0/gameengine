// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RESOURCE_RESOURCE_TYPE_H_
#define COMET_COMET_RESOURCE_RESOURCE_TYPE_H_

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
}  // namespace resource
}  // namespace comet

#endif  // COMET_COMET_RESOURCE_RESOURCE_TYPE_H_