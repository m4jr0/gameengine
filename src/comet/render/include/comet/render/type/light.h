// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_RENDER_TYPE_LIGHT_H_
#define COMET_RENDER_TYPE_LIGHT_H_

#include "comet/core/essentials.h"
#include "comet/entity/type/entity_id.h"
#include "comet/math/vector.h"
#include "comet/rendering/rendering_handle.h"

namespace comet {
namespace rendering {
struct Light {
  bool is_dirty{true};
  entity::EntityId entity_id{entity::kInvalidEntityId};
  LightHandle handle{};
  LightProperties props{};
  LightShadow shadow{};
};
}  // namespace rendering
}  // namespace comet

#endif  // COMET_RENDER_TYPE_LIGHT_H_
