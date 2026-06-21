// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_RUNTIME_LIGHT_LIGHT_H_
#define COMET_RUNTIME_LIGHT_LIGHT_H_

#include "comet/core/essentials.h"
#include "comet/core/math/vector.h"
#include "comet/runtime/entity/entity_id.h"
#include "comet/runtime/light/light_handle.h"

namespace comet {
namespace light {
struct Light {
  bool is_dirty{true};
  entity::EntityId entity_id{entity::kInvalidEntityId};
  LightHandle handle{};
  LightProperties props{};
  LightShadow shadow{};
};
}  // namespace light
}  // namespace comet

#endif  // COMET_RUNTIME_LIGHT_LIGHT_H_
