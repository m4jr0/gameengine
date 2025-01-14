// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_TYPE_MAP_H_
#define COMET_COMET_CORE_TYPE_MAP_H_

#include <functional>
#include <utility>

#include "comet/core/essentials.h"
#include "comet/core/hash.h"
#include "comet/core/memory/allocator/allocator.h"
#include "comet/core/type/hash_set.h"

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
      : key{std::forward<K>(key)}, value{std::forward<V>(value)} {}

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

template <typename K, typename V>
Pair(K&&, V&&) -> Pair<K, V>;

template <typename TKey, typename TValue>
struct MapHashLogic : public SetHashLogic<Pair<TKey, TValue>, TKey> {
  using EntryKey = typename SetHashLogic<Pair<TKey, TValue>, TKey>::Hashable;
  using EntryPair = typename SetHashLogic<Pair<TKey, TValue>, TKey>::Value;
};

namespace internal {
template <typename TKey, typename TValue>
struct DefaultMapHashLogic : public MapHashLogic<TKey, TValue> {
  using EntryPair = typename MapHashLogic<TKey, TValue>::Value;
  using EntryKey = typename MapHashLogic<TKey, TValue>::Hashable;

  static const EntryKey& GetHashable(const EntryPair& pair) { return pair.key; }

  static HashValue Hash(const EntryKey& key) { return GenerateHash(key); }

  static bool AreEqual(const EntryKey& a, const EntryKey& b) { return a == b; }
};
}  // namespace internal

template <typename Key, typename Value,
          typename HashLogic = internal::DefaultMapHashLogic<Key, Value>>
class Map {
 public:
  using KVPair = Pair<Key, Value>;
  using Pairs = HashSet<KVPair, HashLogic>;

  using Iterator = typename Pairs::Iterator;
  using ConstIterator = typename Pairs::ConstIterator;

  Iterator begin() { return pairs_.begin(); }
  Iterator end() { return pairs_.end(); }
  ConstIterator begin() const { return pairs_.begin(); }
  ConstIterator end() const { return pairs_.end(); }
  ConstIterator cbegin() const { return pairs_.cbegin(); }
  ConstIterator cend() const { return pairs_.cend(); }

  static inline constexpr usize kDefaultCapacity{16};

  Map() = default;

  Map(memory::Allocator* allocator)
      : Map(allocator, allocator != nullptr ? kDefaultCapacity : 0) {}

  Map(memory::Allocator* allocator, usize capacity)
      : pairs_{allocator, capacity}, allocator_{allocator} {}

  Map(const Map& other) : pairs_{other.pairs_}, allocator_{other.allocator_} {}

  Map(Map&& other) noexcept
      : pairs_{std::move(other.pairs_)}, allocator_{other.allocator_} {
    other.allocator_ = nullptr;
  }

  Map& operator=(const Map& other) {
    if (this == &other) {
      return *this;
    }

    Clear();
    this->pairs_ = other.pairs_;
    this->allocator_ = other.allocator_;
    return *this;
  }

  Map& operator=(Map&& other) noexcept {
    if (this == &other) {
      return *this;
    }

    Clear();
    this->pairs_ = std::move(other.pairs_);
    this->allocator_ = other.allocator_;

    other.allocator_ = nullptr;
    return *this;
  }

  ~Map() { Clear(); }

  Value& operator[](const Key& key) { return Get(key); }

  bool operator==(const Map& other) const {
    return this->pairs_ == other.pairs_;
  }

  bool operator!=(const Map& other) const { return !(*this == other); }

  Value& Get(const Key& key) {
    auto* value = TryGet(key);

    if (value != nullptr) {
      return *value;
    }

    if constexpr (std::is_constructible_v<Value, memory::Allocator*>) {
      auto& new_pair =
          this->pairs_.Emplace(KVPair{key, Value(this->allocator_)});
      return new_pair.value;
    } else if constexpr (std::is_default_constructible_v<Value>) {
      auto& new_pair = this->pairs_.Emplace(KVPair{key, Value{}});
      return new_pair.value;
    } else {
      COMET_ASSERT(false,
                   "Key not found and value is not default-constructible.");
      throw std::runtime_error(
          "Key not found and value is not default-constructible.");
    }
  }

  const Value& Get(const Key& key) const {
    auto* value = TryGet(key);
    COMET_ASSERT(value != nullptr, "No value found!");
    return *value;
  }

  Value* TryGet(const Key& key) {
    auto* pair{this->pairs_.Find(key)};
    return pair ? &pair->value : nullptr;
  }

  const Value* TryGet(const Key& key) const {
    const auto* pair{this->pairs_.Find(key)};
    return pair ? &pair->value : nullptr;
  }

  template <typename K, typename V>
  void Set(K&& key, V&& value) {
    this->pairs_.Add(KVPair{std::forward<K>(key), std::forward<V>(value)});
  }

  template <typename P>
  void Set(P&& pair) {
    this->pairs_.Add(std::forward<P>(pair));
  }

  template <typename K, typename... Targs>
  KVPair& Emplace(K&& key, Targs&&... args) {
    return this->pairs_.Emplace(
        KVPair{std::forward<K>(key), Value{std::forward<Targs>(args)...}});
  }

  bool Remove(const Key& key) { return this->pairs_.Remove(key); }

  Value Pop(const Key& key) {
    auto pair{this->pairs_.Pop(key)};
    return pair.value;
  }

  void Clear() { this->pairs_.Clear(); }

  bool IsContained(const Key& key) const {
    return this->pairs_.IsContained(key);
  }

  void Reserve(usize capacity) { this->pairs_.Reserve(capacity); }

  usize GetEntryCount() const noexcept { return this->pairs_.GetEntryCount(); }

  usize GetBucketCount() const noexcept {
    return this->pairs_.GetBucketCount();
  }

  bool IsEmpty() const noexcept { return this->pairs_.IsEmpty(); }

 private:
  Pairs pairs_{};
  memory::Allocator* allocator_{nullptr};
};
}  // namespace comet

#endif  // COMET_COMET_CORE_TYPE_MAP_H_