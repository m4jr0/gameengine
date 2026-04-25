// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_ENTITY_MANAGER_IMPL_ENTITY_MANAGER_PENDING_TEMPLATE_H_
#define COMET_COMET_ENTITY_MANAGER_IMPL_ENTITY_MANAGER_PENDING_TEMPLATE_H_

#include "comet/core/essentials.h"

namespace comet {
namespace entity {
template <typename... ComponentTypes>
void EntityManager::DeferAddingComponents(internal::PendingEntity* entity,
                                          const ComponentTypes&... components) {
  (DeferAddingComponent(entity, components), ...);
}

template <typename ComponentType>
void EntityManager::DeferAddingComponent(internal::PendingEntity* entity,
                                         const ComponentType& component) {
  const auto& type_descr{ComponentTypeDescrGetter<ComponentType>::Get()};

  entity->removed_cmps.Remove(type_descr.id);

  for (auto& added : entity->added_cmps) {
    if (added.descr.type_descr.id == type_descr.id) {
      if (type_descr.size > 0) {
        memory::CopyMemory(added.descr.data,
                           reinterpret_cast<const u8*>(&component),
                           type_descr.size);
      }

      return;
    }
  }

  if (type_descr.size == 0) {
    entity->added_cmps.EmplaceLast(internal::ComponentKind::Component,
                                   ComponentDescr{type_descr, nullptr});
    return;
  }

  auto* allocator{GetWritePendingAllocator()};
  auto* data{static_cast<u8*>(
      allocator->AllocateAligned(type_descr.size, type_descr.align))};

  memory::CopyMemory(data, reinterpret_cast<const u8*>(&component),
                     type_descr.size);

  entity->added_cmps.EmplaceLast(internal::ComponentKind::Component,
                                 ComponentDescr{type_descr, data});
}
}  // namespace entity
}  // namespace comet

#endif  // COMET_COMET_ENTITY_MANAGER_IMPL_ENTITY_MANAGER_PENDING_TEMPLATE_H_