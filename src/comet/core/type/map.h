// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_TYPE_MAP_H_
#define COMET_COMET_CORE_TYPE_MAP_H_

#include <atomic>
#include <functional>

#include "comet/core/essentials.h"
#include "comet/core/memory/allocator/allocator.h"
#include "comet/core/type/set.h"

namespace comet {
template <typename Key, typename Value>
struct Pair {
  Key key;
  Value value;

  Pair() = default;

  Pair(const Pair& other) : key{other.key}, value{other.value} {}

  Pair(Pair&& other) noexcept
      : key{std::move(other.key)}, value{std::move(other.value)} {}

  template <typename K, typename V>
  Pair(K&& key, V&& value)
      : key(std::forward<K>(key)), value(std::forward<V>(value)) {}

  Pair& operator=(const Pair& other) {
    if (this == &other) {
      return *this;
    }

    key = other.key;
    value = other.value;
    return *this;
  }

  Pair& operator=(Pair&& other) noexcept {
    if (this == &other) {
      return *this;
    }

    key = std::move(other.key);
    value = std::move(other.value);
    return *this;
  }

  ~Pair() = default;
};

template <typename TKey, typename TValue>
struct MapHashLogic : public SetHashLogic<Pair<TKey, TValue>, TKey> {
  using Key = typename SetHashLogic<Pair<TKey, TValue>, TKey>::Hashable;
  using Pair = typename SetHashLogic<Pair<TKey, TValue>, TKey>::Value;
};

namespace internal {
template <typename TKey, typename TValue>
struct DefaultMapHashLogic : public MapHashLogic<TKey, TValue> {
  using Pair = typename MapHashLogic<TKey, TValue>::Value;
  using Key = typename MapHashLogic<TKey, TValue>::Hashable;

  static const Key& GetHashable(const Pair& pair) { return pair.key; }

  static usize Hash(const Key& key) { return HashCrC32(&key, sizeof(Key)); }

  static bool AreEqual(const Key& a, const Key& b) { return a == b; }
};
}  // namespace internal

template <typename Key, typename Value,
          typename HashLogic = internal::DefaultMapHashLogic<Key, Value>>
class Map {
 public:
  using Pair = Pair<Key, Value>;
  using Pairs = Set<Pair, HashLogic>;

  static inline constexpr usize kDefaultCapacity_{16};

  Map() = default;

  Map(memory::Allocator* allocator, usize capacity = kDefaultCapacity_)
      : pairs_{allocator, capacity} {}

  Map(const Map& other) : pairs_{other.pairs_} {}

  Map(Map&& other) noexcept : pairs_{std::move(other.pairs_)} {}

  Map& operator=(const Map& other) {
    if (this == &other) {
      return *this;
    }

    Destroy();
    this->pairs_ = other.pairs_;
    return *this;
  }

  Map& operator=(Map&& other) noexcept {
    if (this == &other) {
      return *this;
    }

    Destroy();
    this->pairs_ = std::move(other.pairs_);
    return *this;
  }

  ~Map() { Destroy(); }

  void Destroy() { Clear(); }

  Value& operator[](const Key& key) { return Get(key); }

  Value& Get(const Key& key) {
    auto* value{TryGet(key)};
    COMET_ASSERT(value != nullptr, "No value found!");
    return *value;
  }

  Value* TryGet(const Key& key) {
    auto* pair{this->pairs_.Find(key)};
    return pair ? &pair->value : nullptr;
  }

  // >:3 Emplace?
  bool Set(const Key& key, const Value& value) {
    return this->pairs_.Add(Pair{key, value});
  }

  void Clear() { this->pairs_.Clear(); }

  bool IsContained(const Key& key) const {
    return this->pairs_.IsContained(key);
  }

  usize GetSize() const noexcept { return this->pairs_.GetSize(); }

  usize GetCapacity() const noexcept { return this->pairs_.GetCapacity(); }

 private:
  Pairs pairs_{};
};

// Implemented from https://nullprogram.com/blog/2023/09/30/
template <typename Key, typename Value,
          typename HashLogic = internal::DefaultMapHashLogic<Key, Value>>
class LockFreeMap {
 public:
  using Pair = Pair<Key, Value>;
  using Hashable = typename HashLogic::Hashable;

  static inline constexpr usize kDefaultCapacity_{16};

  LockFreeMap() = default;

  LockFreeMap(memory::Allocator* allocator, usize capacity = kDefaultCapacity_)
      : capacity_{capacity}, allocator_{allocator} {
    this->table_ = allocator_->AllocateMany<std::atomic<Node*>>(capacity_);

    for (usize i{0}; i < capacity_; ++i) {
      this->table_[i].store(nullptr, std::memory_order_relaxed);
    }
  }

  LockFreeMap(const LockFreeMap& other) = delete;

  LockFreeMap(LockFreeMap&& other) noexcept
      : capacity_{other.capacity_},
        size_{other.size_.load(std::memory_order_acquire)},
        data_{other.data_},
        allocator_{other.allocator_} {
    other.capacity_ = 0;
    other.size_.store(0, std::memory_order_release);
    other.data_ = nullptr;
    other.allocator_ = nullptr;
  }

  LockFreeMap& operator=(const LockFreeMap& other) = delete;

  LockFreeMap& operator=(LockFreeMap&& other) noexcept {
    if (this == &other) {
      return *this;
    }

    Destroy();

    this->capacity_ = other.capacity_;
    this->size_.store(other.size_.load(std::memory_order_acquire),
                      std::memory_order_relaxed);
    this->data_ = other.data_;
    this->allocator_ = other.allocator_;

    other.capacity_ = 0;
    other.size_.store(0, std::memory_order_release);
    other.data_ = nullptr;
    other.allocator_ = nullptr;
    return *this;
  }

  ~LockFreeMap() { Destroy(); }

  void Destroy() {
    Clear();

    this->allocator_->Deallocate(this->data_);
    this->capacity_ = 0;
    this->size_.store(0, std::memory_order_release);
    this->data_ = nullptr;
    this->allocator_ = nullptr;
  }

  Value& operator[](const Key& key) { return Get(key); }

  Value& Get(const Key& key) {
    auto* value{TryGet(key)};
    COMET_ASSERT(value != nullptr, "No value found!");
    return *value;
  }

  Value* TryGet(const Key& key) {
    auto index{GetBucketIndex(HashLogic::GetHashable(key))};
    auto* bucket{this->data_[index]};

    for (;;) {
      auto* node{bucket.load(std::memory_order_acquire)};

      if (node == nullptr) {
        return nullptr;
      }

      auto& pair{node->pair};

      if (HashLogic::AreEqual(pair.key, key)) {
        return pair.value;
      }

      node = node->next;
    }

    return nullptr;
  }

  void Set(const Key& key, const Value& value) {
    auto index{GetBucketIndex(HashLogic::GetHashable(key))};
    auto* bucket{data_[index]};

    for (;;) {
      auto* node{bucket.load(std::memory_order_acquire)};

      if (node == nullptr) {
        auto* new_node{this->allocator_->AllocateOne<Node>(Pair{key, value})};

        if (bucket->compare_exchange_weak(node, new_node,
                                          std::memory_order_release,
                                          std::memory_order_acquire)) {
          this->size_.fetch_add(1, std::memory_order_relaxed);
          return true;
        }

        this->allocator_->Deallocate(new_node);
      }

      auto& pair{node->pair};

      if (HashLogic::AreEqual(pair.key, key)) {
        pair.value = value;
        return false;
      }

      node = node->next;
    }

    return false;
  }

  void Clear() {
    for (usize i{0}; i < this->capacity_; ++i) {
      auto* node{this->data_[i].load(std::memory_order_acquire)};

      while (node != nullptr) {
        auto* next{node->next};
        this->allocator_->Deallocate(node);
        node = next;
      }

      this->data_[i].store(nullptr, std::memory_order_relaxed);
    }

    this->size_.store(0, std::memory_order_relaxed);
  }

  bool IsContained(const Key& key) const { return TryGet(key) != nullptr; }

  usize GetSize() const noexcept {
    return this->size_.load(std::memory_order_relaxed);
  }

  usize GetCapacity() const noexcept { return this->capacity_; }

 private:
  struct Node {
    Pair pair{};
    Node* next{nullptr};
  };

  using Bucket = std::atomic<Node*>;

  usize GetBucketIndex(const Hashable& hashable) const {
    return HashLogic::Hash(hashable) % this->capacity_;
  }

  usize capacity_{0};
  std::atomic<usize> size_{0};
  Bucket* data_{nullptr};
  memory::Allocator* allocator_{nullptr};
};
// >:3
// https://nullprogram.com/blog/2022/08/08/
// https://nrk.neocities.org/articles/hash-trees-and-tries
// https://github.com/skeeto/lstack/blob/master/lstack.c
}  // namespace comet

#endif  // COMET_COMET_CORE_TYPE_MAP_H_