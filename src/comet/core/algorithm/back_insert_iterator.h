// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_ALGORITHM_BACK_INSERT_ITERATOR_H_
#define COMET_COMET_CORE_ALGORITHM_BACK_INSERT_ITERATOR_H_

// External. ///////////////////////////////////////////////////////////////////
#include <utility>
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/essentials.h"

namespace comet {
template <typename Container>
class BackInsertIterator {
 public:
  using ContainerType = Container;

  explicit BackInsertIterator(Container& container) : container_{&container} {}

  BackInsertIterator& operator=(
      const typename Container::Iterator::value_type& value) {
    container_->EmplaceBack(value);
    return *this;
  }

  BackInsertIterator& operator=(
      typename Container::Iterator::value_type&& value) {
    container_->EmplaceBack(std::move(value));
    return *this;
  }

  BackInsertIterator& operator*() noexcept { return *this; }
  BackInsertIterator& operator++() noexcept { return *this; }
  BackInsertIterator& operator++(int) noexcept { return *this; }

 private:
  Container* container_{nullptr};
};

template <typename Container>
BackInsertIterator<Container> BackInserter(Container& container) {
  return BackInsertIterator<Container>{container};
}
}  // namespace comet

#endif  // COMET_COMET_CORE_ALGORITHM_BACK_INSERT_ITERATOR_H_
