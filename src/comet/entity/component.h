// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_ENTITY_COMPONENT_H_
#define COMET_COMET_ENTITY_COMPONENT_H_

#include "comet_precompile.h"

#include "comet/entity/entity_id.h"

namespace comet {
namespace entity {
template <typename ComponentType>
struct is_component {
  static constexpr auto value{
      std::is_trivially_copyable_v<std::decay_t<ComponentType>>};
};

template <class ComponentType>
constexpr bool is_component_v{is_component<ComponentType>::value};

template <typename ComponentType>
constexpr void CheckComponent() {
  static_assert(is_component_v<ComponentType>,
                "Component must be trivially copyable!");
}

struct ComponentTypeDescr {
  EntityId id{kInvalidEntityId};
  uindex size{0};
  u16 alignment{0};
};

class ComponentIdGenerator {
 protected:
  inline static EntityId GenerateId() {
    COMET_ASSERT(component_id_counter_ < kMaxComponentId - 1,
                 "Max component count reached!");

    return Tag(EntityIdTag::Component, component_id_counter_++);
  }

 private:
  inline static u8 component_id_counter_{0};
};

template <typename ComponentType>
class ComponentTypeDescrGetter : public ComponentIdGenerator {
 public:
  static const ComponentTypeDescr& Get() {
    if (is_descr_generated) {
      return descr_;
    }

    descr_.id = GenerateId();
    descr_.size = sizeof(ComponentType);
    descr_.alignment = alignof(ComponentType);
    is_descr_generated = true;
    return descr_;
  }

 private:
  inline static ComponentTypeDescr descr_{};
  inline static auto is_descr_generated{false};
};

struct ComponentDescr {
  ComponentTypeDescr type_descr{};
  const u8* data{nullptr};
};
}  // namespace entity
}  // namespace comet

#endif  // COMET_COMET_ENTITY_COMPONENT_H_
