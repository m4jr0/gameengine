// Copyright 2022 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_ENTITY_COMPONENT_COMPONENT_H_
#define COMET_COMET_ENTITY_COMPONENT_COMPONENT_H_

#include "comet_precompile.h"

#include "comet/entity/entity.h"

namespace comet {
namespace entity {
using ComponentTypeId = stringid::StringId;
constexpr auto kInvalidComponentTypeId{kInvalidEntityId};
}  // namespace entity
}  // namespace comet

#endif  // COMET_COMET_ENTITY_COMPONENT_COMPONENT_H_
