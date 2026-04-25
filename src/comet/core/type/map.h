// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_TYPE_MAP_H_
#define COMET_COMET_CORE_TYPE_MAP_H_

// External. ///////////////////////////////////////////////////////////////////
#include <stdexcept>
#include <type_traits>
#include <utility>
////////////////////////////////////////////////////////////////////////////////

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

template <typename TValue>
struct DefaultMapHashLogic<const schar*, TValue>
    : public MapHashLogic<const schar*, TValue> {
  using EntryPair = typename MapHashLogic<const schar*, TValue>::Value;
  using EntryKey = typename MapHashLogic<const schar*, TValue>::Hashable;

  static const EntryKey& GetHashable(const EntryPair& pair) { return pair.key; }

  static HashValue Hash(const EntryKey& key) { return GenerateHash(key); }

  static bool AreEqual(const EntryKey& a, const EntryKey& b) {
    return AreStringsEqual(a, b);
  }
};

template <typename TValue>
struct DefaultMapHashLogic<schar*, TValue>
    : DefaultMapHashLogic<const schar*, TValue> {};

template <typename TValue>
struct DefaultMapHashLogic<const wchar*, TValue>
    : public MapHashLogic<const wchar*, TValue> {
  using EntryPair = typename MapHashLogic<const wchar*, TValue>::Value;
  using EntryKey = typename MapHashLogic<const wchar*, TValue>::Hashable;

  static const EntryKey& GetHashable(const EntryPair& pair) { return pair.key; }

  static HashValue Hash(const EntryKey& key) { return GenerateHash(key); }

  static bool AreEqual(const EntryKey& a, const EntryKey& b) {
    return AreStringsEqual(a, b);
  }
};

template <typename TValue>
struct DefaultMapHashLogic<wchar*, TValue>
    : DefaultMapHashLogic<const wchar*, TValue> {};
}  // namespace internal

template <typename Key, typename Value,
          typename HashLogic = internal::DefaultMapHashLogic<Key, Value>>
class Map {
  struct WithCapacityTag {};

 public:
  using KVPair = Pair<Key, Value>;
  using Pairs = HashSet<KVPair, HashLogic>;

  using Iterator = typename Pairs::Iterator;
  using ConstIterator = typename Pairs::ConstIterator;

  static inline constexpr usize kDefaultCapacity{16};

  static Map WithCapacity(memory::Allocator* allocator, usize capacity) {
    return Map{allocator, WithCapacityTag{}, capacity};
  }

  Map() = default;

  explicit Map(memory::Allocator* allocator)
      : pairs_{allocator}, allocator_{allocator} {}

  Map(const Map& other) : pairs_{other.pairs_}, allocator_{other.allocator_} {}

  Map(Map&& other) noexcept
      : pairs_{std::move(other.pairs_)}, allocator_{other.allocator_} {
    other.allocator_ = nullptr;
  }

  Map& operator=(const Map& other) {
    if (this == &other) {
      return *this;
    }

    Release();

    pairs_ = other.pairs_;
    allocator_ = other.allocator_;
    return *this;
  }

  Map& operator=(Map&& other) noexcept {
    if (this == &other) {
      return *this;
    }

    Release();

    pairs_ = std::move(other.pairs_);
    allocator_ = other.allocator_;

    other.allocator_ = nullptr;
    return *this;
  }

  ~Map() { Release(); }

  Iterator begin() { return pairs_.begin(); }

  Iterator end() { return pairs_.end(); }

  ConstIterator begin() const { return pairs_.begin(); }

  ConstIterator end() const { return pairs_.end(); }

  ConstIterator cbegin() const { return pairs_.cbegin(); }

  ConstIterator cend() const { return pairs_.cend(); }

  bool operator==(const Map& other) const { return pairs_ == other.pairs_; }

  bool operator!=(const Map& other) const { return !(*this == other); }

  void Release() {
    pairs_.Release();
    allocator_ = nullptr;
  }

  Value& Get(const Key& key) {
    auto* value{TryGet(key)};
    COMET_ASSERT(value != nullptr, "Map::Get", "key not found");
    return *value;
  }

  const Value& Get(const Key& key) const {
    const auto* value{TryGet(key)};
    COMET_ASSERT(value != nullptr, "Map::Get", "key not found");
    return *value;
  }

  Value* TryGet(const Key& key) {
    auto* pair{pairs_.Find(key)};
    return pair != nullptr ? &pair->value : nullptr;
  }

  const Value* TryGet(const Key& key) const {
    const auto* pair{pairs_.Find(key)};
    return pair != nullptr ? &pair->value : nullptr;
  }

  Value& GetOrAdd(const Key& key) {
    auto* value{TryGet(key)};

    if (value != nullptr) {
      return *value;
    }

    return EmplaceDefaultValue(key).value;
  }

  template <typename... Targs>
  Value& GetOrAdd(const Key& key, Targs&&... args) {
    auto* value{TryGet(key)};

    if (value != nullptr) {
      return *value;
    }

    auto& pair{
        pairs_.Emplace(KVPair{key, Value{std::forward<Targs>(args)...}})};
    return pair.value;
  }

  template <typename K, typename V>
  void Set(K&& key, V&& value) {
    pairs_.Set(KVPair{std::forward<K>(key), std::forward<V>(value)});
  }

  template <typename P>
  void Set(P&& pair) {
    pairs_.Set(std::forward<P>(pair));
  }

  template <typename K, typename... Targs>
  KVPair& Emplace(K&& key, Targs&&... args) {
    return pairs_.Emplace(
        KVPair{std::forward<K>(key), Value{std::forward<Targs>(args)...}});
  }

  bool Remove(const Key& key) { return pairs_.Remove(key); }

  template <typename Predicate>
  usize RemoveIf(Predicate&& predicate) {
    return pairs_.RemoveIf(
        [&](const KVPair& pair) { return predicate(pair.key, pair.value); });
  }

  Value Pop(const Key& key) {
    auto pair{pairs_.Pop(key)};
    return std::move(pair.value);
  }

  void Clear() { pairs_.Clear(); }

  bool IsContained(const Key& key) const { return pairs_.IsContained(key); }

  void Reserve(usize capacity) { pairs_.Reserve(capacity); }

  void TrimCapacity() { pairs_.TrimCapacity(); }

  void SetMaxLoadFactor(f32 max_load_factor) {
    pairs_.SetMaxLoadFactor(max_load_factor);
  }

  f32 GetMaxLoadFactor() const noexcept { return pairs_.GetMaxLoadFactor(); }

  usize GetEntryCount() const noexcept { return pairs_.GetEntryCount(); }

  usize GetBucketCount() const noexcept { return pairs_.GetBucketCount(); }

  bool IsEmpty() const noexcept { return pairs_.IsEmpty(); }

  memory::Allocator* GetAllocator() noexcept { return allocator_; }

  const memory::Allocator* GetAllocator() const noexcept { return allocator_; }

 private:
  Map(memory::Allocator* allocator, WithCapacityTag, usize capacity)
      : pairs_{Pairs::WithCapacity(allocator, capacity)},
        allocator_{allocator} {
    COMET_ASSERT(capacity == 0 || allocator != nullptr, "Map::WithCapacity",
                 "allocator is null");
  }

  KVPair& EmplaceDefaultValue(const Key& key) {
    if constexpr (std::is_constructible_v<Value, memory::Allocator*>) {
      return pairs_.Emplace(KVPair{key, Value{allocator_}});
    } else if constexpr (std::is_default_constructible_v<Value>) {
      return pairs_.Emplace(KVPair{key, Value{}});
    } else {
      COMET_ASSERT(false, "Map::GetOrAdd",
                   "value is not default-constructible");
      throw std::runtime_error("value is not default-constructible");
    }
  }

  Pairs pairs_{};
  memory::Allocator* allocator_{nullptr};
};
}  // namespace comet

#endif  // COMET_COMET_CORE_TYPE_MAP_H_