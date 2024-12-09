// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_TYPE_SET_H_
#define COMET_COMET_CORE_TYPE_SET_H_

#include <functional>

#include "comet/core/essentials.h"
#include "comet/core/hash.h"
#include "comet/core/memory/allocator/allocator.h"
#include "comet/core/type/array.h"

namespace comet {
template <typename T, typename THashable>
struct SetHashLogic {
  using Value = T;
  using Hashable = THashable;
};

namespace internal {
template <typename T>
struct DefaultSetHashLogic : public SetHashLogic<T, T> {
  using Value = typename SetHashLogic<T, T>::Value;
  using Hashable = typename SetHashLogic<T, T>::Hashable;

  static const typename Hashable& GetHashable(const Value& obj) { return obj; }

  static usize Hash(const Hashable& hashable) {
    return HashCrC32(&hashable, sizeof(Hashable));
  }

  static bool AreEqual(const Hashable& a, const Hashable& b) { return a == b; }
};
}  // namespace internal

template <typename T, typename HashLogic = internal::DefaultSetHashLogic<T>>
class Set {
 public:
  using Bucket = Array<T>;
  using Buckets = Array<Bucket>;
  using Hashable = typename HashLogic::Hashable;

  static inline constexpr usize kDefaultObjCount{16};

  Set() = default;

  Set(memory::Allocator* allocator, usize default_obj_count = kDefaultObjCount)
      : buckets_{allocator}, allocator_{allocator} {
    if (default_obj_count == 0) {
      return;
    }

    Reserve(default_obj_count);
  }

  Set(const Set& other)
      : max_load_factor_{other.max_load_factor_},
        capacity_{other.capacity_},
        size_{other.size_},
        buckets_{other.buckets_},
        allocator_{other.allocator_} {}

  Set(Set&& other) noexcept
      : max_load_factor_{other.max_load_factor_},
        capacity_{std::move(other.capacity_)},
        size_{std::move(other.size_)},
        buckets_{std::move(other.buckets_)},
        allocator_{other.allocator_} {
    other.max_load_factor_ = 1.0f;
    other.capacity_ = 0;
    other.size_ = 0;
    other.allocator_ = nullptr;
  }

  Set& operator=(const Set& other) {
    if (this == &other) {
      return *this;
    }

    this->max_load_factor_ = other.max_load_factor_;
    this->capacity_ = other.capacity_;
    this->size_ = other.size_;
    this->buckets_ = other.buckets_;
    this->allocator_ = other.allocator_;

    return *this;
  }

  Set& operator=(Set&& other) {
    if (this == &other) {
      return *this;
    }

    this->max_load_factor_ = other.max_load_factor_;
    this->capacity_ = other.capacity_;
    this->size_ = other.size_;
    this->buckets_ = std::move(other.buckets_);
    this->allocator_ = other.allocator_;

    other.max_load_factor_ = 1.0f;
    other.capacity_ = 0;
    other.size_ = 0;
    other.allocator_ = nullptr;
    return *this;
  }

  bool Add(const T& obj) {
    auto& hashable{HashLogic::GetHashable(obj)};
    auto index{GetBucketIndex(hashable)};

    for (const auto& existing_obj : this->buckets_[index]) {
      if (HashLogic::AreEqual(HashLogic::GetHashable(existing_obj), hashable)) {
        return false;
      }
    }

    this->buckets_[index].EmplaceBack(obj);
    ++this->size_;
    return true;
  }

  bool Remove(const T& obj) {
    auto index{GetBucketIndex(HashLogic::GetHashable(obj))};
    auto& bucket{this->buckets_[index]};

    for (auto it{bucket.begin()}; it != bucket.end(); ++it) {
      if (HashLogic::AreEqual(*it, obj)) {
        bucket.Remove(it);
        --this->size_;
        return true;
      }
    }

    return false;
  }

  void Clear() {
    this->buckets_.Clear();
    this->size_ = 0;
    this->capacity_ = 0;
  }

  T* Find(const Hashable& hashable) {
    auto index{GetBucketIndex(hashable)};

    for (auto& obj : this->buckets_[index]) {
      if (HashLogic::AreEqual(HashLogic::GetHashable(obj), hashable)) {
        return &obj;
      }
    }

    return nullptr;
  }

  void Reserve(usize min_obj_count) {
    auto new_capacity{static_cast<usize>(min_obj_count / max_load_factor_)};

    if (new_capacity <= this->capacity_) {
      return;
    }

    Rehash(new_capacity);
  }

  bool IsContained(const T& obj) const {
    auto index{GetBucketIndex(obj)};

    for (const auto& existing_obj : this->buckets_[index]) {
      if (HashLogic::AreEqual(existing_obj, obj)) {
        return true;
      }
    }

    return false;
  }

  void SetMaxLoadFactor(f32 max_load_factor) {
    this->max_load_factor_ = max_load_factor;
  }

  const usize GetSize() const noexcept { return this->size_; }

  const usize GetCapacity() const noexcept { return this->capacity_; }

  const f32 GetMaxLoadFactor() const noexcept { return this->max_load_factor_; }

 private:
  usize GetBucketIndex(const Hashable& hashable) const {
    return HashLogic::Hash(hashable) % this->capacity_;
  }

  void Rehash(usize new_capacity = kInvalidSize) {
    if (new_capacity == kInvalidSize) {
      new_capacity = this->capacity_;
    }

    Buckets new_buckets{this->allocator_};
    new_buckets.Resize(new_capacity);

    for (usize i{0}; i < this->capacity_; ++i) {
      for (const auto& obj : this->buckets_[i]) {
        auto new_index{HashLogic::Hash(HashLogic::GetHashable(obj)) %
                       new_capacity};
        new_buckets[new_index].EmplaceBack(obj);
      }
    }

    buckets_ = std::move(new_buckets);
    capacity_ = new_capacity;
  }

  f32 max_load_factor_{1.0f};
  usize capacity_{0};
  usize size_{0};
  Buckets buckets_{};
  memory::Allocator* allocator_{nullptr};
};
}  // namespace comet

#endif  // COMET_COMET_CORE_TYPE_SET_H_