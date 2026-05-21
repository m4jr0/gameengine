// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_RUNTIME_ENTITY_MANAGER_IMPL_ENTITY_MANAGER_DATA_TEMPLATE_H_
#define COMET_RUNTIME_ENTITY_MANAGER_IMPL_ENTITY_MANAGER_DATA_TEMPLATE_H_

#include "comet/core/essentials.h"

namespace comet {
namespace entity {
template <typename... ComponentTypes, typename... ComponentTypeIds>
auto EntityManager::BuildSortedComponentIdArray(
    ComponentTypeIds... component_type_ids) {
  constexpr auto component_type_count{sizeof...(ComponentTypes)};
  constexpr auto component_type_ids_count{sizeof...(ComponentTypeIds)};
  constexpr auto component_id_count{component_type_count +
                                    component_type_ids_count};

  StaticArray<EntityId, component_id_count> all_ids{};
  usize i{0};
  (void(all_ids[i++] = component_type_ids), ...);
  (void(all_ids[i++] = ComponentTypeDescrGetter<ComponentTypes>::Get().id),
   ...);
  std::sort(all_ids.begin(), all_ids.end());
  return all_ids;
}
}  // namespace entity
}  // namespace comet

#endif  // COMET_RUNTIME_ENTITY_MANAGER_IMPL_ENTITY_MANAGER_DATA_TEMPLATE_H_