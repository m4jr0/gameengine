// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_TYPE_ORDERED_SET_H_
#define COMET_COMET_CORE_TYPE_ORDERED_SET_H_

#include <utility>

#include "comet/core/essentials.h"
#include "comet/core/type/array.h"
#include "comet/core/type/hash_set.h"
#include "comet/core/type/iterator.h"

namespace comet {
template <typename T, typename HashLogic = internal::DefaultSetHashLogic<T>>
class OrderedSet {
 public:
  using Elements = Array<T>;
  using Hashes = HashSet<u32>;

  COMET_POPULATE_ITERATOR(T, elements_.GetData(), elements_.GetSize())

  OrderedSet() = default;

  explicit OrderedSet(memory::Allocator* allocator)
      : OrderedSet{allocator, kDefaultCapacity_} {}

  OrderedSet(memory::Allocator* allocator, usize capacity)
      : elements_{allocator, capacity}, hashes_{allocator, capacity} {}

  OrderedSet(const OrderedSet& other)
      : elements_{other.elements_}, hashes_{other.hashes_} {}

  OrderedSet(OrderedSet&& other) noexcept
      : elements_{std::move(other.elements_)},
        hashes_{std::move(other.hashes_)} {}

  OrderedSet& operator=(const OrderedSet& other) {
    if (this == &other) {
      return *this;
    }

    this->elements_ = other.elements_;
    this->hashes_ = other.hashes_;
    return *this;
  }

  OrderedSet& operator=(OrderedSet&& other) noexcept {
    if (this == &other) {
      return *this;
    }

    this->elements_ = std::move(other.elements_);
    this->hashes_ = std::move(other.hashes_);
    return *this;
  }

  ~OrderedSet() { Destroy(); }

  void Destroy() {
    this->elements_.Destroy();
    this->hashes_.Destroy();
  }

  template <typename V>
  void Add(V&& value) {
    auto hash{HashLogic::Hash(HashLogic::GetHashable(value))};

    if (this->hashes_.IsContained(hash)) {
      return;
    }

    this->elements_.PushBack(std::forward<V>(value));
    this->hashes_.Add(hash);
  }

  template <typename... Targs>
  T& EmplaceBack(Targs&&... args) {
    T value{std::forward<Targs>(args)...};
    auto hash{HashLogic::Hash(HashLogic::GetHashable(value))};

    if (this->hashes_.IsContained(hash)) {
      return this->elements_.Get(this->elements_.GetIndex(value));
    }

    this->hashes_.Add(hash);
    return this->elements_.EmplaceBack(std::move(value));
  }

  bool Remove(const T& value) {
    auto hash{HashLogic::Hash(HashLogic::GetHashable(value))};

    if (!this->hashes_.IsContained(hash)) {
      return false;
    }

    auto index{this->elements_.GetIndex(value)};

    if (index == kInvalidIndex) {
      return false;
    }

    this->elements_.RemoveFromIndex(index);
    this->hashes_.Remove(hash);
    return true;
  }

  T& operator[](usize index) { return this->elements_.Get(index); }

  const T& operator[](usize index) const { return this->elements_.Get(index); }

  T& Get(usize index) { return this->elements_.Get(index); }

  const T& Get(usize index) const { return this->elements_.Get(index); }

  void Reserve(usize capacity) {
    this->elements_.Reserve(capacity);
    this->hashes_.Reserve(capacity);
  }

  void Clear() {
    this->elements_.Clear();
    this->hashes_.Clear();
  }

  bool IsContained(const T& value) const {
    return hashes_.IsContained(HashLogic::Hash(HashLogic::GetHashable(value)));
  }

  void SetMaxLoadFactor(f32 max_load_factor) {
    this->hashes_.SetMaxLoadFactor(max_load_factor);
  }

  usize GetSize() const noexcept { return this->elements_.GetSize(); }

  bool IsEmpty() const noexcept { return this->elements_.IsEmpty(); }

  usize GetCapacity() const noexcept { return this->elements_.GetCapacity(); }

  f32 GetMaxLoadFactor() const noexcept {
    return this->hashes_.GetMaxLoadFactor();
  }

 private:
  static inline constexpr usize kDefaultCapacity_{16};

  Elements elements_{};
  Hashes hashes_{};
};
}  // namespace comet

#endif  // COMET_COMET_CORE_TYPE_ORDERED_SET_H_