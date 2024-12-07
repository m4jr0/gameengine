// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_TYPE_MAP_H_
#define COMET_COMET_CORE_TYPE_MAP_H_

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
struct MapFuncs : public SetFuncs<Pair<TKey, TValue>, TKey> {
  using Key = typename SetFuncs<Pair<TKey, TValue>, TKey>::Hashable;
  using Pair = typename SetFuncs<Pair<TKey, TValue>, TKey>::Value;
};

namespace internal {
template <typename TKey, typename TValue>
struct DefaultMapFuncs : public MapFuncs<TKey, TValue> {
  using Pair = typename MapFuncs<TKey, TValue>::Value;
  using Key = typename MapFuncs<TKey, TValue>::Hashable;

  static const Key& GetHashable(const Pair& pair) { return pair.key; }

  static usize Hash(const Key& key) { return HashCrC32(&key, sizeof(Key)); }

  static bool AreEqual(const Key& a, const Key& b) { return a == b; }
};
}  // namespace internal

template <typename Key, typename Value,
          typename MapFuncs = internal::DefaultMapFuncs<Key, Value>>
class Map {
 public:
  using Pair = Pair<Key, Value>;
  using Pairs = Set<Pair, MapFuncs>;

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

    Clear();
    this->pairs_ = other.pairs_;
    return *this;
  }

  Map& operator=(Map&& other) noexcept {
    if (this == &other) {
      return *this;
    }

    Clear();
    this->pairs_ = std::move(other.pairs_);
    return *this;
  }

  ~Map() { Clear(); }

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
}  // namespace comet

#endif  // COMET_COMET_CORE_TYPE_MAP_H_