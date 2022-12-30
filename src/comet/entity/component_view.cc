// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "component_view.h"

namespace comet {
namespace entity {
ComponentView::ConstIterator::ConstIterator(std::vector<Archetype*> archetypes,
                                            uindex archetype_index,
                                            uindex entity_index)
    : archetypes_{std::move(archetypes)},
      archetype_index_{archetype_index},
      entity_index_{entity_index} {}

const ComponentView::ConstIterator::reference
ComponentView::ConstIterator::operator*() const {
  return archetypes_[archetype_index_]->entity_ids[entity_index_];
}

const ComponentView::ConstIterator::pointer
ComponentView::ConstIterator::operator->() {
  return &archetypes_[archetype_index_]->entity_ids[entity_index_];
}

ComponentView::ConstIterator& ComponentView::ConstIterator::operator++() {
  uindex archetype_count{archetypes_.size()};

  if (entity_index_ < archetypes_[archetype_index_]->entity_ids.size() - 1) {
    ++entity_index_;
    return *this;
  } else if (archetype_index_ < archetype_count - 1) {
    do {
      ++archetype_index_;
    } while (archetype_index_ < archetype_count &&
             archetypes_[archetype_index_]->entity_ids.size() == 0);

    if (archetype_index_ < archetype_count) {
      entity_index_ = 0;
      return *this;
    }
  }

  archetype_index_ = kInvalidIndex;
  entity_index_ = kInvalidIndex;
  return *this;
}

ComponentView::ConstIterator ComponentView::ConstIterator::operator++(int) {
  ComponentView::ConstIterator tmp{*this};
  ++(*this);
  return tmp;
}

bool operator==(const ComponentView::ConstIterator& a,
                const ComponentView::ConstIterator& b) {
  return a.archetype_index_ == b.archetype_index_ &&
         a.entity_index_ == b.entity_index_;
}

bool operator!=(const ComponentView::ConstIterator& a,
                const ComponentView::ConstIterator& b) {
  return !operator==(a, b);
}

const ComponentView::ConstIterator ComponentView::begin() const {
  if (archetypes_.size() == 0) {
    return ComponentView::ConstIterator{archetypes_, kInvalidIndex,
                                        kInvalidIndex};
  }

  uindex archetype_index{0};

  do {
    if (archetypes_[archetype_index]->entity_ids.size() > 0) {
      return ComponentView::ConstIterator{archetypes_, archetype_index, 0};
    }

    ++archetype_index;
  } while (archetype_index < archetypes_.size());

  return ComponentView::ConstIterator{archetypes_, kInvalidIndex,
                                      kInvalidIndex};
}

const ComponentView::ConstIterator ComponentView::end() const {
  return ComponentView::ConstIterator{archetypes_, kInvalidIndex,
                                      kInvalidIndex};
}

const ComponentView::ConstIterator ComponentView::cbegin() const {
  if (archetypes_.size() == 0) {
    return ComponentView::ConstIterator{archetypes_, kInvalidIndex,
                                        kInvalidIndex};
  }

  uindex archetype_index{0};

  do {
    if (archetypes_[archetype_index]->entity_ids.size() > 0) {
      return ComponentView::ConstIterator{archetypes_, archetype_index, 0};
    }

    ++archetype_index;
  } while (archetype_index < archetypes_.size());

  return ComponentView::ConstIterator{archetypes_, kInvalidIndex,
                                      kInvalidIndex};
}

const ComponentView::ConstIterator ComponentView::cend() const {
  return ComponentView::ConstIterator{archetypes_, kInvalidIndex,
                                      kInvalidIndex};
}

ComponentView::ComponentView(ComponentTypeId component_type_ids[],
                             uindex component_count,
                             const std::vector<Archetype*>& archetypes) {
  if (component_count == 0) {
    archetypes_ = archetypes;
    return;
  }

  std::sort(component_type_ids, component_type_ids + component_count);

  for (Archetype* archetype : archetypes) {
    if (archetype->entity_type.size() < component_count) {
      continue;
    }

    const auto result{std::search(
        archetype->entity_type.cbegin(), archetype->entity_type.cend(),
        component_type_ids, component_type_ids + component_count)};

    if (result == archetype->entity_type.cend()) {
      continue;
    }

    archetypes_.emplace_back(archetype);
  }
}
}  // namespace entity
}  // namespace comet
