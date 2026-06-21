// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_RUNTIME_ENTITY_PENDING_ENTITY_H_
#define COMET_RUNTIME_ENTITY_PENDING_ENTITY_H_

#include "comet/core/essentials.h"
#include "comet/runtime/memory/allocator/stack_allocator.h"
#include "comet/core/container/array.h"
#include "comet/core/container/ordered_set.h"
#include "comet/runtime/entity/component.h"
#include "comet/runtime/entity/entity_id.h"

namespace comet {
namespace entity {
namespace internal {
using EntityPendingAllocator = memory::FiberStackAllocator;

enum class ComponentKind { Unknown = 0, Component, Parent };

struct AddedComponent {
  ComponentKind kind{};
  ComponentDescr descr{};
};

struct PendingEntity {
  bool is_destroyed{false};
  EntityId id{kInvalidEntityId};
  Array<AddedComponent> added_cmps{};
  OrderedSet<EntityId> removed_cmps{};
};
}  // namespace internal
}  // namespace entity
}  // namespace comet

#endif  // COMET_RUNTIME_ENTITY_PENDING_ENTITY_H_
