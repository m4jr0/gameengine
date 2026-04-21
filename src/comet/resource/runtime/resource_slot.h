// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RESOURCE_RUNTIME_RESOURCE_SLOT_H_
#define COMET_COMET_RESOURCE_RUNTIME_RESOURCE_SLOT_H_

#include "comet/core/essentials.h"
#include "comet/resource/resource_id.h"
#include "comet/resource/type/resource_common_type.h"

namespace comet {
namespace resource {
template <typename Tag, typename T>
struct ResourceSlot {
  ResourceIdT<Tag> id{};
  ResourceLifeSpan life_span{ResourceLifeSpan::Unknown};
  usize ref_count{0};
  T* resource{nullptr};
};

template <typename Tag, typename T>
constexpr bool IsValid(const ResourceSlot<Tag, T>& slot) noexcept {
  return slot.id.IsValid() && slot.resource != nullptr;
}
}  // namespace resource
}  // namespace comet

#endif  // COMET_COMET_RESOURCE_RUNTIME_RESOURCE_SLOT_H_