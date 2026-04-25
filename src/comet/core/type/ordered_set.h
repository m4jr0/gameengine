// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_TYPE_ORDERED_SET_H_
#define COMET_COMET_CORE_TYPE_ORDERED_SET_H_

// External. ///////////////////////////////////////////////////////////////////
#include <utility>
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/essentials.h"
#include "comet/core/type/array.h"
#include "comet/core/type/hash_set.h"
#include "comet/core/type/iterator.h"
#include "comet/core/type/map.h"

namespace comet {
template <typename T, typename HashLogic = internal::DefaultSetHashLogic<T>>
class OrderedSet {
  struct WithCapacityTag {};

 public:
  using Elements = Array<T>;
  using Indices = Array<usize>;
  using Lookup = Map<HashValue, Indices>;

  COMET_POPULATE_ITERATOR(T, elements_.GetData(), elements_.GetSize())

  static OrderedSet WithCapacity(memory::Allocator* allocator, usize capacity) {
    return OrderedSet{allocator, WithCapacityTag{}, capacity};
  }

  OrderedSet() = default;

  explicit OrderedSet(memory::Allocator* allocator)
      : elements_{allocator}, lookup_{allocator}, allocator_{allocator} {}

  OrderedSet(const OrderedSet& other)
      : elements_{other.elements_},
        lookup_{other.lookup_},
        allocator_{other.allocator_} {}

  OrderedSet(OrderedSet&& other) noexcept
      : elements_{std::move(other.elements_)},
        lookup_{std::move(other.lookup_)},
        allocator_{other.allocator_} {
    other.allocator_ = nullptr;
  }

  OrderedSet& operator=(const OrderedSet& other) {
    if (this == &other) {
      return *this;
    }

    Release();
    elements_ = other.elements_;
    lookup_ = other.lookup_;
    allocator_ = other.allocator_;
    return *this;
  }

  OrderedSet& operator=(OrderedSet&& other) noexcept {
    if (this == &other) {
      return *this;
    }

    Release();
    elements_ = std::move(other.elements_);
    lookup_ = std::move(other.lookup_);
    allocator_ = other.allocator_;
    other.allocator_ = nullptr;
    return *this;
  }

  ~OrderedSet() { Release(); }

  void Release() {
    elements_.Release();
    lookup_.Release();
    allocator_ = nullptr;
  }

  template <typename V>
  void Add(V&& value) {
    if (FindIndex(value) != kInvalidIndex) {
      return;
    }

    const auto hash{HashLogic::Hash(HashLogic::GetHashable(value))};
    const auto index{elements_.GetSize()};

    elements_.PushLast(std::forward<V>(value));
    GetOrCreateBucket(hash).PushLast(index);
  }

  template <typename... Targs>
  T& EmplaceLast(Targs&&... args) {
    T value{std::forward<Targs>(args)...};

    const auto existing_index{FindIndex(value)};

    if (existing_index != kInvalidIndex) {
      return elements_.Get(existing_index);
    }

    const auto hash{HashLogic::Hash(HashLogic::GetHashable(value))};
    const auto index{elements_.GetSize()};

    GetOrCreateBucket(hash).PushLast(index);
    return elements_.EmplaceLast(std::move(value));
  }

  bool Remove(const T& value) {
    const auto hash{HashLogic::Hash(HashLogic::GetHashable(value))};
    auto* bucket{lookup_.TryGet(hash)};

    if (bucket == nullptr) {
      return false;
    }

    for (usize i{0}; i < bucket->GetSize(); ++i) {
      const auto index{bucket->Get(i)};

      if (HashLogic::AreEqual(HashLogic::GetHashable(elements_.Get(index)),
                              HashLogic::GetHashable(value))) {
        bucket->RemoveFromIndex(i);

        if (bucket->IsEmpty()) {
          lookup_.Remove(hash);
        }

        elements_.RemoveFromIndex(index);
        DecrementIndicesAfter(index);
        return true;
      }
    }

    return false;
  }

  T& operator[](usize index) { return elements_.Get(index); }
  const T& operator[](usize index) const { return elements_.Get(index); }

  T& Get(usize index) { return elements_.Get(index); }
  const T& Get(usize index) const { return elements_.Get(index); }

  T& GetFirst() { return elements_.GetFirst(); }
  const T& GetFirst() const { return elements_.GetFirst(); }

  T& GetLast() { return elements_.GetLast(); }
  const T& GetLast() const { return elements_.GetLast(); }

  void Reserve(usize capacity) {
    elements_.Reserve(capacity);
    lookup_.Reserve(capacity);
  }

  void TrimCapacity() {
    elements_.TrimCapacity();
    lookup_.TrimCapacity();
  }

  void Clear() {
    elements_.Clear();
    lookup_.Clear();
  }

  bool IsContained(const T& value) const {
    return FindIndex(value) != kInvalidIndex;
  }

  bool Contains(const T& value) const { return IsContained(value); }

  void SetMaxLoadFactor(f32 max_load_factor) {
    lookup_.SetMaxLoadFactor(max_load_factor);
  }

  usize GetSize() const noexcept { return elements_.GetSize(); }
  bool IsEmpty() const noexcept { return elements_.IsEmpty(); }
  usize GetCapacity() const noexcept { return elements_.GetCapacity(); }
  f32 GetMaxLoadFactor() const noexcept { return lookup_.GetMaxLoadFactor(); }

 private:
  OrderedSet(memory::Allocator* allocator, WithCapacityTag, usize capacity)
      : elements_{Elements::WithCapacity(allocator, capacity)},
        lookup_{Lookup::WithCapacity(allocator, capacity)},
        allocator_{allocator} {
    COMET_ASSERT(capacity == 0 || allocator != nullptr,
                 "OrderedSet::WithCapacity", "allocator is null");
  }

  Indices& GetOrCreateBucket(HashValue hash) {
    auto* bucket{lookup_.TryGet(hash)};

    if (bucket != nullptr) {
      return *bucket;
    }

    lookup_.Set(hash, Indices{allocator_});
    return lookup_.Get(hash);
  }

  usize FindIndex(const T& value) const {
    const auto hash{HashLogic::Hash(HashLogic::GetHashable(value))};
    const auto* bucket{lookup_.TryGet(hash)};

    if (bucket == nullptr) {
      return kInvalidIndex;
    }

    for (const auto index : *bucket) {
      COMET_ASSERT(index < elements_.GetSize(), "OrderedSet::FindIndex",
                   "lookup index out of bounds", "index", index, "size",
                   elements_.GetSize());

      if (HashLogic::AreEqual(HashLogic::GetHashable(elements_.Get(index)),
                              HashLogic::GetHashable(value))) {
        return index;
      }
    }

    return kInvalidIndex;
  }

  void DecrementIndicesAfter(usize removed_index) {
    for (auto& pair : lookup_) {
      for (usize& index : pair.value) {
        if (index > removed_index) {
          --index;
        }
      }
    }
  }

  Elements elements_{};
  Lookup lookup_{};
  memory::Allocator* allocator_{nullptr};
};
}  // namespace comet

#endif  // COMET_COMET_CORE_TYPE_ORDERED_SET_H_