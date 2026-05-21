// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_CORE_CONTAINER_HASH_SET_H_
#define COMET_CORE_CONTAINER_HASH_SET_H_

// External. ///////////////////////////////////////////////////////////////////
#include <iterator>
#include <type_traits>
#include <utility>
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/c_string.h"
#include "comet/core/essentials.h"
#include "comet/core/hash.h"
#include "comet/core/memory/allocator/allocator.h"
#include "comet/core/type/array.h"
#include "comet/core/type/traits.h"
#include "comet/math/math_scalar.h"

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

  static HashValue Hash(const EntryHashable& hashable) {
    return GenerateHash(hashable);
  }

  static bool AreEqual(const EntryHashable& a, const EntryHashable& b) {
    return a == b;
  }
};

template <>
struct DefaultSetHashLogic<const schar*> {
  using Value = const schar*;
  using Hashable = const schar*;

  static const Hashable& GetHashable(const Value& obj) { return obj; }

  static HashValue Hash(const Hashable& str) { return GenerateHash(str); }

  static bool AreEqual(const Hashable& a, const Hashable& b) {
    return AreStringsEqual(a, b);
  }
};

template <>
struct DefaultSetHashLogic<schar*> : DefaultSetHashLogic<const schar*> {};

template <>
struct DefaultSetHashLogic<const wchar*> {
  using Value = const wchar*;
  using Hashable = const wchar*;

  static const Hashable& GetHashable(const Value& obj) { return obj; }

  static HashValue Hash(const Hashable& str) { return GenerateHash(str); }

  static bool AreEqual(const Hashable& a, const Hashable& b) {
    return AreStringsEqual(a, b);
  }
};

template <>
struct DefaultSetHashLogic<wchar*> : DefaultSetHashLogic<const wchar*> {};
}  // namespace internal

template <typename T, typename HashLogic = internal::DefaultSetHashLogic<T>>
class HashSet {
  struct WithCapacityTag {};

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
      const auto tmp{*this};
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

  static inline constexpr usize kDefaultObjCount{16};

  static HashSet WithCapacity(memory::Allocator* allocator, usize capacity) {
    return HashSet{allocator, WithCapacityTag{}, capacity};
  }

  HashSet() = default;

  explicit HashSet(memory::Allocator* allocator)
      : buckets_{allocator}, allocator_{allocator} {}

  HashSet(const HashSet& other)
      : max_load_factor_{other.max_load_factor_},
        bucket_count_{other.bucket_count_},
        entry_count_{other.entry_count_},
        buckets_{other.buckets_},
        allocator_{other.allocator_} {}

  HashSet(HashSet&& other) noexcept
      : max_load_factor_{other.max_load_factor_},
        bucket_count_{other.bucket_count_},
        entry_count_{other.entry_count_},
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

    Release();

    max_load_factor_ = other.max_load_factor_;
    bucket_count_ = other.bucket_count_;
    entry_count_ = other.entry_count_;
    buckets_ = other.buckets_;
    allocator_ = other.allocator_;

    return *this;
  }

  HashSet& operator=(HashSet&& other) noexcept {
    if (this == &other) {
      return *this;
    }

    Release();

    max_load_factor_ = other.max_load_factor_;
    bucket_count_ = other.bucket_count_;
    entry_count_ = other.entry_count_;
    buckets_ = std::move(other.buckets_);
    allocator_ = other.allocator_;

    other.max_load_factor_ = 1.0f;
    other.bucket_count_ = 0;
    other.entry_count_ = 0;
    other.allocator_ = nullptr;

    return *this;
  }

  ~HashSet() { Release(); }

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

  bool operator==(const HashSet& other) const {
    if (entry_count_ != other.entry_count_) {
      return false;
    }

    for (const auto& bucket : buckets_) {
      for (const auto& entry : bucket) {
        if (!other.IsContained(HashLogic::GetHashable(entry))) {
          return false;
        }
      }
    }

    return true;
  }

  bool operator!=(const HashSet& other) const { return !(*this == other); }

  void Release() {
    buckets_.Release();
    entry_count_ = 0;
    bucket_count_ = 0;
    allocator_ = nullptr;
    max_load_factor_ = 1.0f;
  }

  template <typename V>
  void Add(V&& obj) {
    CheckSize();
    auto& hashable{HashLogic::GetHashable(obj)};

    const auto index{GetBucketIndex(hashable)};
    COMET_ASSERT(index != kInvalidIndex, "HashSet::Add",
                 "hash set is unallocated");

    auto& bucket{buckets_[index]};

    for (const auto& existing_obj : bucket) {
      if (HashLogic::AreEqual(HashLogic::GetHashable(existing_obj), hashable)) {
        return;
      }
    }

    bucket.PushLast(std::forward<V>(obj));
    ++entry_count_;
  }

  template <typename V>
  void Set(V&& obj) {
    CheckSize();
    auto& hashable{HashLogic::GetHashable(obj)};

    const auto index{GetBucketIndex(hashable)};
    COMET_ASSERT(index != kInvalidIndex, "HashSet::Set",
                 "hash set is unallocated");

    auto& bucket{buckets_[index]};

    for (usize i{0}; i < bucket.GetSize(); ++i) {
      const auto& existing_obj{bucket[i]};

      if (HashLogic::AreEqual(HashLogic::GetHashable(existing_obj), hashable)) {
        bucket[i] = std::forward<V>(obj);
        return;
      }
    }

    bucket.PushLast(std::forward<V>(obj));
    ++entry_count_;
  }

  template <typename... Targs>
  T& Emplace(Targs&&... args) {
    CheckSize();

    T obj{std::forward<Targs>(args)...};
    auto& hashable{HashLogic::GetHashable(obj)};

    const auto index{GetBucketIndex(hashable)};
    COMET_ASSERT(index != kInvalidIndex, "HashSet::Emplace",
                 "hash set is unallocated");

    auto& bucket{buckets_[index]};

    for (auto& existing_obj : bucket) {
      if (HashLogic::AreEqual(HashLogic::GetHashable(existing_obj), hashable)) {
        return existing_obj;
      }
    }

    bucket.EmplaceLast(std::move(obj));
    ++entry_count_;
    return bucket.GetLast();
  }

  bool Remove(const Hashable& hashable) {
    const auto index{GetBucketIndex(hashable)};

    if (index == kInvalidIndex) {
      return false;
    }

    auto& bucket{buckets_[index]};
    const auto bucket_size{bucket.GetSize()};

    for (usize i{0}; i < bucket_size; ++i) {
      if (HashLogic::AreEqual(HashLogic::GetHashable(bucket[i]), hashable)) {
        bucket.RemoveFromIndex(i);
        --entry_count_;
        return true;
      }
    }

    return false;
  }

  template <typename Predicate>
  usize RemoveIf(Predicate&& predicate) {
    usize removed_count{0};

    for (auto& bucket : buckets_) {
      for (usize i{0}; i < bucket.GetSize();) {
        if (predicate(bucket[i])) {
          bucket.RemoveFromIndex(i);
          --entry_count_;
          ++removed_count;
        } else {
          ++i;
        }
      }
    }

    return removed_count;
  }

  void Clear() {
    for (auto& bucket : buckets_) {
      bucket.Clear();
    }

    entry_count_ = 0;
  }

  T* Find(const Hashable& hashable) {
    const auto index{GetBucketIndex(hashable)};

    if (index == kInvalidIndex) {
      return nullptr;
    }

    for (auto& obj : buckets_[index]) {
      if (HashLogic::AreEqual(HashLogic::GetHashable(obj), hashable)) {
        return &obj;
      }
    }

    return nullptr;
  }

  const T* Find(const Hashable& hashable) const {
    const auto index{GetBucketIndex(hashable)};

    if (index == kInvalidIndex) {
      return nullptr;
    }

    for (const auto& obj : buckets_[index]) {
      if (HashLogic::AreEqual(HashLogic::GetHashable(obj), hashable)) {
        return &obj;
      }
    }

    return nullptr;
  }

  T Pop(const Hashable& hashable) {
    T popped{};

    const auto index{GetBucketIndex(hashable)};
    COMET_ASSERT(index != kInvalidIndex, "HashSet::Pop",
                 "hash set is unallocated");

    auto& bucket{buckets_[index]};

    for (usize i{0}; i < bucket.GetSize(); ++i) {
      if (HashLogic::AreEqual(HashLogic::GetHashable(bucket[i]), hashable)) {
        popped = std::move(bucket[i]);
        bucket.RemoveFromIndex(i);
        --entry_count_;
        return popped;
      }
    }

    COMET_ASSERT(false, "HashSet::Pop", "value not found", "bucket_index",
                 index);
    return popped;
  }

  bool IsContained(const Hashable& hashable) const {
    const auto index{GetBucketIndex(hashable)};

    if (index == kInvalidIndex) {
      return false;
    }

    for (const auto& existing_obj : buckets_[index]) {
      if (HashLogic::AreEqual(HashLogic::GetHashable(existing_obj), hashable)) {
        return true;
      }
    }

    return false;
  }

  void Reserve(usize entry_count) {
    COMET_ASSERT(allocator_ != nullptr, "HashSet::Reserve",
                 "allocator is null");

    const auto bucket_count{
        static_cast<usize>(math::Ceil(entry_count / max_load_factor_))};

    if (bucket_count <= bucket_count_) {
      return;
    }

    Rehash(bucket_count);
  }

  void TrimCapacity() {
    for (auto& bucket : buckets_) {
      bucket.TrimCapacity();
    }
  }

  void SetMaxLoadFactor(f32 max_load_factor) {
    COMET_ASSERT(max_load_factor > .0f, "HashSet::SetMaxLoadFactor",
                 "max load factor must be positive", "max_load_factor",
                 max_load_factor);

    max_load_factor_ = max_load_factor;

    if (entry_count_ > bucket_count_ * max_load_factor_) {
      Reserve(entry_count_);
    }
  }

  usize GetEntryCount() const noexcept { return entry_count_; }

  usize GetBucketCount() const noexcept { return bucket_count_; }

  bool IsEmpty() const noexcept { return entry_count_ == 0; }

  f32 GetMaxLoadFactor() const noexcept { return max_load_factor_; }

  memory::Allocator* GetAllocator() noexcept { return allocator_; }

  const memory::Allocator* GetAllocator() const noexcept { return allocator_; }

 private:
  HashSet(memory::Allocator* allocator, WithCapacityTag, usize capacity)
      : buckets_{allocator}, allocator_{allocator} {
    COMET_ASSERT(capacity == 0 || allocator != nullptr, "HashSet::WithCapacity",
                 "allocator is null");

    if (capacity > 0) {
      Reserve(capacity);
    }
  }

  void CheckSize() {
    if (bucket_count_ == 0) {
      Reserve(kDefaultObjCount);
      return;
    }

    if (entry_count_ + 1 > bucket_count_ * max_load_factor_) {
      Reserve(entry_count_ == 0 ? kDefaultObjCount : entry_count_ * 2);
    }
  }

  usize GetBucketIndex(const Hashable& hashable) const {
    if (bucket_count_ == 0) {
      return kInvalidIndex;
    }

    return static_cast<usize>(HashLogic::Hash(hashable) % bucket_count_);
  }

  void Rehash(usize bucket_count = kInvalidSize) {
    if (bucket_count == kInvalidSize) {
      bucket_count = bucket_count_;
    }

    Buckets new_buckets{allocator_};
    new_buckets.Resize(bucket_count);

    for (usize i{0}; i < bucket_count_; ++i) {
      for (auto& obj : buckets_[i]) {
        const auto new_index{static_cast<usize>(
            HashLogic::Hash(HashLogic::GetHashable(obj)) % bucket_count)};

        if constexpr (std::is_move_constructible_v<T>) {
          new_buckets[new_index].EmplaceLast(std::move(obj));
        } else if constexpr (std::is_copy_constructible_v<T>) {
          new_buckets[new_index].EmplaceLast(obj);
        } else {
          static_assert(always_false_v<T>,
                        "object type must be movable or copyable");
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

#endif  // COMET_CORE_CONTAINER_HASH_SET_H_