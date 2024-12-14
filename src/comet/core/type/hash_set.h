// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_TYPE_HASH_SET_H_
#define COMET_COMET_CORE_TYPE_HASH_SET_H_

#include <functional>
#include <type_traits>

#include "comet/core/essentials.h"
#include "comet/core/hash.h"
#include "comet/core/memory/allocator/allocator.h"
#include "comet/core/type/array.h"
#include "comet/core/type/traits.h"
#include "comet/math/math_commons.h"

namespace comet {
template <typename T, typename THashable>
struct SetHashLogic {
  using Value = T;
  using Hashable = THashable;
};

namespace internal {
template <typename T>
struct DefaultSetHashLogic : public SetHashLogic<T, T> {
  using EntryValue = typename SetHashLogic<T, T>::Value;
  using EntryHashable = typename SetHashLogic<T, T>::Hashable;

  static const EntryHashable& GetHashable(const EntryValue& obj) { return obj; }

  static usize Hash(const EntryHashable& hashable) {
    return GenerateHash(hashable);
  }

  static bool AreEqual(const EntryHashable& a, const EntryHashable& b) {
    return a == b;
  }
};
}  // namespace internal

template <typename T, typename HashLogic = internal::DefaultSetHashLogic<T>>
class HashSet {
 public:
  using Bucket = Array<T>;
  using Buckets = Array<Bucket>;
  using Hashable = typename HashLogic::Hashable;

  template <bool IsConst>
  class IteratorImpl {
   public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = T;
    using pointer = std::conditional_t<IsConst, const T*, T*>;
    using reference = std::conditional_t<IsConst, const T&, T&>;

    using BucketIterator =
        std::conditional_t<IsConst, typename Buckets::ConstIterator,
                           typename Buckets::Iterator>;
    using EntryIterator =
        std::conditional_t<IsConst, typename Bucket::ConstIterator,
                           typename Bucket::Iterator>;

    IteratorImpl() = default;

    IteratorImpl(BucketIterator bucket_it, BucketIterator bucket_end,
                 EntryIterator entry_it, usize entry_count)
        : bucket_it_{entry_count == 0 ? bucket_end : bucket_it},
          bucket_end_{bucket_end},
          entry_it_{entry_it} {
      if (entry_count > 0) {
        SkipEmptyBuckets();
      }
    }

    reference operator*() const noexcept { return *entry_it_; }

    pointer operator->() const noexcept { return entry_it_; }

    IteratorImpl& operator++() {
      ++entry_it_;

      if (entry_it_ == bucket_it_->end()) {
        ++bucket_it_;
        SkipEmptyBuckets();
      }

      return *this;
    }

    IteratorImpl operator++(int) {
      auto tmp{*this};
      ++(*this);
      return tmp;
    }

    bool operator==(const IteratorImpl& other) const noexcept {
      return bucket_it_ == other.bucket_it_ &&
             (bucket_it_ == bucket_end_ || entry_it_ == other.entry_it_);
    }

    bool operator!=(const IteratorImpl& other) const noexcept {
      return !(*this == other);
    }

   private:
    BucketIterator bucket_it_{};
    BucketIterator bucket_end_{};
    EntryIterator entry_it_{};

    void SkipEmptyBuckets() {
      while (bucket_it_ != bucket_end_ &&
             bucket_it_->begin() == bucket_it_->end()) {
        ++bucket_it_;
      }

      if (bucket_it_ != bucket_end_) {
        entry_it_ = bucket_it_->begin();
      }
    }
  };

  using Iterator = IteratorImpl<false>;
  using ConstIterator = IteratorImpl<true>;

  Iterator begin() {
    return Iterator{
        buckets_.begin(), buckets_.end(),
        buckets_.IsEmpty() ? typename Bucket::Iterator{} : buckets_[0].begin(),
        entry_count_};
  }

  Iterator end() {
    return Iterator{buckets_.end(), buckets_.end(), typename Bucket::Iterator{},
                    entry_count_};
  }

  ConstIterator begin() const {
    return ConstIterator{buckets_.begin(), buckets_.end(),
                         buckets_.IsEmpty() ? typename Bucket::ConstIterator{}
                                            : buckets_[0].begin(),
                         entry_count_};
  }

  ConstIterator end() const {
    return ConstIterator{buckets_.end(), buckets_.end(),
                         typename Bucket::ConstIterator{}, entry_count_};
  }

  ConstIterator cbegin() const { return begin(); }
  ConstIterator cend() const { return end(); }

  static inline constexpr usize kDefaultObjCount{16};

  HashSet() = default;

  HashSet(memory::Allocator* allocator,
          usize default_obj_count = kDefaultObjCount)
      : buckets_{allocator}, allocator_{allocator} {
    if (default_obj_count == 0) {
      return;
    }

    Reserve(default_obj_count);
  }

  HashSet(const HashSet& other)
      : max_load_factor_{other.max_load_factor_},
        bucket_count_{other.bucket_count_},
        entry_count_{other.entry_count_},
        buckets_{other.buckets_},
        allocator_{other.allocator_} {}

  HashSet(HashSet&& other) noexcept
      : max_load_factor_{other.max_load_factor_},
        bucket_count_{std::move(other.bucket_count_)},
        entry_count_{std::move(other.entry_count_)},
        buckets_{std::move(other.buckets_)},
        allocator_{other.allocator_} {
    other.max_load_factor_ = 1.0f;
    other.bucket_count_ = 0;
    other.entry_count_ = 0;
    other.allocator_ = nullptr;
  }

  HashSet& operator=(const HashSet& other) {
    if (this == &other) {
      return *this;
    }

    this->max_load_factor_ = other.max_load_factor_;
    this->bucket_count_ = other.bucket_count_;
    this->entry_count_ = other.entry_count_;
    this->buckets_ = other.buckets_;
    this->allocator_ = other.allocator_;

    return *this;
  }

  HashSet& operator=(HashSet&& other) noexcept {
    if (this == &other) {
      return *this;
    }

    this->max_load_factor_ = other.max_load_factor_;
    this->bucket_count_ = other.bucket_count_;
    this->entry_count_ = other.entry_count_;
    this->buckets_ = std::move(other.buckets_);
    this->allocator_ = other.allocator_;

    other.max_load_factor_ = 1.0f;
    other.bucket_count_ = 0;
    other.entry_count_ = 0;
    other.allocator_ = nullptr;
    return *this;
  }

  bool operator==(const HashSet& other) const {
    if (this->entry_count_ != other.entry_count_) {
      return false;
    }

    for (const auto& bucket : this->buckets_) {
      for (const auto& entry : bucket) {
        if (!other.IsContained(HashLogic::GetHashable(entry))) {
          return false;
        }
      }
    }

    return true;
  }

  bool operator!=(const HashSet& other) const { return !(*this == other); }

  template <typename V>
  void Add(V&& obj) {
    CheckSize();
    auto& hashable{HashLogic::GetHashable(obj)};
    auto index{GetBucketIndex(hashable)};
    COMET_ASSERT(index != kInvalidIndex, "Map appears to be unallocated!");

    for (const auto& existing_obj : this->buckets_[index]) {
      if (HashLogic::AreEqual(HashLogic::GetHashable(existing_obj), hashable)) {
        return;
      }
    }

    this->buckets_[index].PushBack(std::forward<V>(obj));
    ++this->entry_count_;
  }

  template <typename... Targs>
  T& Emplace(Targs&&... args) {
    CheckSize();
    T obj{std::forward<Targs>(args)...};
    auto& hashable{HashLogic::GetHashable(obj)};
    auto index{GetBucketIndex(hashable)};
    COMET_ASSERT(index != kInvalidIndex, "Map appears to be unallocated!");
    auto& bucket{this->buckets_[index]};

    for (auto& existing_obj : bucket) {
      if (HashLogic::AreEqual(HashLogic::GetHashable(existing_obj), hashable)) {
        return existing_obj;
      }
    }

    bucket.EmplaceBack(std::move(obj));
    ++this->entry_count_;
    return bucket.GetLast();
  }

  bool Remove(const Hashable& hashable) {
    auto index{GetBucketIndex(hashable)};

    if (index == kInvalidIndex) {
      return false;
    }

    auto& bucket{this->buckets_[index]};
    auto bucket_size{bucket.GetSize()};

    for (usize i{0}; i < bucket_size; ++i) {
      if (HashLogic::AreEqual(HashLogic::GetHashable(bucket[i]), hashable)) {
        bucket.RemoveFromIndex(i);
        --this->entry_count_;
        return true;
      }
    }

    return false;
  }

  void Clear() {
    this->buckets_.Clear();
    this->entry_count_ = 0;
    this->bucket_count_ = 0;
  }

  T* Find(const Hashable& hashable) {
    auto index{GetBucketIndex(hashable)};

    if (index == kInvalidIndex) {
      return nullptr;
    }

    for (auto& obj : this->buckets_[index]) {
      if (HashLogic::AreEqual(HashLogic::GetHashable(obj), hashable)) {
        return &obj;
      }
    }

    return nullptr;
  }

  const T* Find(const Hashable& hashable) const {
    auto index{GetBucketIndex(hashable)};

    if (index == kInvalidIndex) {
      return nullptr;
    }

    for (auto& obj : this->buckets_[index]) {
      if (HashLogic::AreEqual(HashLogic::GetHashable(obj), hashable)) {
        return &obj;
      }
    }

    return nullptr;
  }

  T Pop(const Hashable& hashable) {
    T popped;
    auto index{GetBucketIndex(hashable)};
    COMET_ASSERT(index != kInvalidIndex, "HashSet is unallocated!");
    auto& bucket{buckets_[index]};

    for (usize i{0}; i < bucket.GetSize(); ++i) {
      if (HashLogic::AreEqual(HashLogic::GetHashable(bucket[i]), hashable)) {
        popped = std::move(bucket[i]);
        bucket.RemoveFromIndex(i);
        --entry_count_;
        return popped;
      }
    }

    COMET_ASSERT(false, "Value not found in HashSet!");
    return popped;
  }

  void Reserve(usize entry_count) {
    auto bucket_count{
        static_cast<usize>(math::Ceil(entry_count / max_load_factor_))};

    if (bucket_count <= this->bucket_count_) {
      return;
    }

    Rehash(bucket_count);
  }

  void CheckSize() {
    if (this->entry_count_ + 1 > this->bucket_count_ * max_load_factor_) {
      Reserve(this->entry_count_ * 2);
    }
  }

  bool IsContained(const Hashable& hashable) const {
    auto index{GetBucketIndex(hashable)};

    if (index == kInvalidIndex) {
      return false;
    }

    for (const auto& existing_obj : this->buckets_[index]) {
      if (HashLogic::AreEqual(HashLogic::GetHashable(existing_obj), hashable)) {
        return true;
      }
    }

    return false;
  }

  void SetMaxLoadFactor(f32 max_load_factor) {
    this->max_load_factor_ = max_load_factor;
  }

  usize GetEntryCount() const noexcept { return this->entry_count_; }

  usize GetBucketCount() const noexcept { return this->bucket_count_; }

  f32 GetMaxLoadFactor() const noexcept { return this->max_load_factor_; }

 private:
  usize GetBucketIndex(const Hashable& hashable) const {
    if (this->bucket_count_ == 0) {
      return kInvalidIndex;
    }

    return HashLogic::Hash(hashable) % this->bucket_count_;
  }

  void Rehash(usize bucket_count = kInvalidSize) {
    if (bucket_count == kInvalidSize) {
      bucket_count = this->bucket_count_;
    }

    Buckets new_buckets{this->allocator_};
    new_buckets.Resize(bucket_count);

    for (usize i{0}; i < this->bucket_count_; ++i) {
      for (auto& obj : this->buckets_[i]) {
        auto new_index{HashLogic::Hash(HashLogic::GetHashable(obj)) %
                       bucket_count};
        if constexpr (std::is_move_constructible_v<decltype(obj)>) {
          new_buckets[new_index].EmplaceBack(std::move(obj));
        } else if constexpr (std::is_copy_constructible_v<decltype(obj)>) {
          new_buckets[new_index].EmplaceBack(obj);
        } else {
          static_assert(always_false_v<decltype(obj)>,
                        "Object type must be moveable or copyable!");
        }
      }
    }

    buckets_ = std::move(new_buckets);
    bucket_count_ = bucket_count;
  }

  f32 max_load_factor_{1.0f};
  usize bucket_count_{0};
  usize entry_count_{0};
  Buckets buckets_{};
  memory::Allocator* allocator_{nullptr};
};
}  // namespace comet

#endif  // COMET_COMET_CORE_TYPE_HASH_SET_H_