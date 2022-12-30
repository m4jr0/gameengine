// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_ENTITY_COMPONENT_VIEW_H_
#define COMET_COMET_ENTITY_COMPONENT_VIEW_H_

#include "comet_precompile.h"

#include "comet/entity/component/component.h"
#include "comet/entity/entity_misc_types.h"

namespace comet {
namespace entity {
class ComponentView {
 public:
  class ConstIterator {
    using iterator_category = std::forward_iterator_tag;
    using difference_type = sptrdiff;
    using value_type = EntityId;
    using pointer = value_type*;
    using reference = value_type&;

   public:
    ConstIterator(std::vector<Archetype*> archetypes,
                  uindex archetype_index = 0, uindex entity_index = 0);
    ConstIterator(const ConstIterator&) = default;
    ConstIterator(ConstIterator&&) = default;
    ConstIterator& operator=(const ConstIterator&) = default;
    ConstIterator& operator=(ConstIterator&&) = default;
    ~ConstIterator() = default;

    const reference operator*() const;
    const pointer operator->();
    ConstIterator& operator++();
    ConstIterator operator++(int);
    friend bool operator==(const ConstIterator& a, const ConstIterator& b);
    friend bool operator!=(const ConstIterator& a, const ConstIterator& b);

   private:
    std::vector<Archetype*> archetypes_{};
    uindex archetype_index_{0};
    uindex entity_index_{0};
  };

  template <typename... ComponentTypes>
  static ComponentView Generate(const std::vector<Archetype*>& archetypes) {
    ComponentTypeId component_type_ids[]{{ComponentTypes::kComponentTypeId...}};

    return ComponentView{component_type_ids, sizeof...(ComponentTypes),
                         archetypes};
  };

  const ConstIterator begin() const;
  const ConstIterator end() const;
  const ConstIterator cbegin() const;
  const ConstIterator cend() const;
  ComponentView(ComponentTypeId component_type_ids[], uindex component_count,
                const std::vector<Archetype*>& archetypes);
  ComponentView(const ComponentView&) = delete;
  ComponentView(ComponentView&&) = delete;
  ComponentView& operator=(const ComponentView&) = delete;
  ComponentView& operator=(ComponentView&&) = delete;
  ~ComponentView() = default;

 private:
  std::vector<Archetype*> archetypes_{};
};
}  // namespace entity
}  // namespace comet

#endif  // COMET_COMET_ENTITY_COMPONENT_VIEW_H_
