// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_FRAME_FRAME_CONTAINER_H_
#define COMET_COMET_CORE_FRAME_FRAME_CONTAINER_H_

#include "comet/core/essentials.h"
#include "comet/core/frame/frame_allocator.h"
#include "comet/core/type/array.h"
#include "comet/core/type/bitset.h"
#include "comet/core/type/hash_set.h"
#include "comet/core/type/map.h"
#include "comet/core/type/ordered_set.h"

namespace comet {
namespace frame {
template <typename T>
class FrameArray : public Array<T> {
 public:
  static FrameArray WithCapacity(usize capacity) {
    return FrameArray{capacity};
  }

  static FrameArray FromData(const T* data, usize count) {
    return FrameArray{FromDataTag{}, data, count};
  }

  FrameArray() : Array<T>{&GetFrameAllocator()} {}

  explicit FrameArray(usize capacity)
      : Array<T>{Array<T>::WithCapacity(&GetFrameAllocator(), capacity)} {}

  template <typename... Targs,
            typename = std::enable_if_t<(sizeof...(Targs) > 1)>>
  FrameArray(Targs&&... args)
      : Array<T>{&GetFrameAllocator(), std::forward<Targs>(args)...} {}

  FrameArray(const FrameArray&) = delete;
  FrameArray& operator=(const FrameArray&) = delete;
  FrameArray(FrameArray&&) noexcept = default;
  FrameArray& operator=(FrameArray&&) noexcept = default;

 private:
  struct FromDataTag {};

  FrameArray(FromDataTag, const T* data, usize count)
      : Array<T>{Array<T>::FromData(&GetFrameAllocator(), data, count)} {}
};

template <typename T>
class DoubleFrameArray : public Array<T> {
 public:
  static DoubleFrameArray WithCapacity(usize capacity) {
    return DoubleFrameArray{capacity};
  }

  static DoubleFrameArray FromData(const T* data, usize count) {
    return DoubleFrameArray{FromDataTag{}, data, count};
  }

  DoubleFrameArray() : Array<T>{&GetDoubleFrameAllocator()} {}

  explicit DoubleFrameArray(usize capacity)
      : Array<T>{Array<T>::WithCapacity(&GetDoubleFrameAllocator(), capacity)} {
  }

  template <typename... Targs,
            typename = std::enable_if_t<(sizeof...(Targs) > 1)>>
  DoubleFrameArray(Targs&&... args)
      : Array<T>{&GetDoubleFrameAllocator(), std::forward<Targs>(args)...} {}

  DoubleFrameArray(const DoubleFrameArray&) = delete;
  DoubleFrameArray& operator=(const DoubleFrameArray&) = delete;
  DoubleFrameArray(DoubleFrameArray&&) noexcept = default;
  DoubleFrameArray& operator=(DoubleFrameArray&&) noexcept = default;

 private:
  struct FromDataTag {};

  DoubleFrameArray(FromDataTag, const T* data, usize count)
      : Array<T>{Array<T>::FromData(&GetDoubleFrameAllocator(), data, count)} {}
};

class FrameBitset : public Bitset {
 public:
  static FrameBitset WithSize(usize bit_count) {
    return FrameBitset{bit_count};
  }

  FrameBitset() : Bitset{&GetFrameAllocator()} {}

  explicit FrameBitset(usize bit_count)
      : Bitset{Bitset::WithSize(&GetFrameAllocator(), bit_count)} {}

  FrameBitset(const FrameBitset&) = delete;
  FrameBitset& operator=(const FrameBitset&) = delete;
  FrameBitset(FrameBitset&&) noexcept = default;
  FrameBitset& operator=(FrameBitset&&) noexcept = default;
};

class DoubleFrameBitset : public Bitset {
 public:
  static DoubleFrameBitset WithSize(usize bit_count) {
    return DoubleFrameBitset{bit_count};
  }

  DoubleFrameBitset() : Bitset{&GetDoubleFrameAllocator()} {}

  explicit DoubleFrameBitset(usize bit_count)
      : Bitset{Bitset::WithSize(&GetDoubleFrameAllocator(), bit_count)} {}

  DoubleFrameBitset(const DoubleFrameBitset&) = delete;
  DoubleFrameBitset& operator=(const DoubleFrameBitset&) = delete;
  DoubleFrameBitset(DoubleFrameBitset&&) noexcept = default;
  DoubleFrameBitset& operator=(DoubleFrameBitset&&) noexcept = default;
};

template <typename T, typename HashLogic = internal::DefaultSetHashLogic<T>>
class FrameHashSet : public HashSet<T, HashLogic> {
 public:
  static FrameHashSet WithCapacity(usize capacity) {
    return FrameHashSet{capacity};
  }

  FrameHashSet() : HashSet<T, HashLogic>{&GetFrameAllocator()} {}

  explicit FrameHashSet(usize capacity)
      : HashSet<T, HashLogic>{HashSet<T, HashLogic>::WithCapacity(
            &GetFrameAllocator(), capacity)} {}

  FrameHashSet(const FrameHashSet&) = delete;
  FrameHashSet& operator=(const FrameHashSet&) = delete;
  FrameHashSet(FrameHashSet&&) noexcept = default;
  FrameHashSet& operator=(FrameHashSet&&) noexcept = default;
};

template <typename T, typename HashLogic = internal::DefaultSetHashLogic<T>>
class DoubleFrameHashSet : public HashSet<T, HashLogic> {
 public:
  static DoubleFrameHashSet WithCapacity(usize capacity) {
    return DoubleFrameHashSet{capacity};
  }

  DoubleFrameHashSet() : HashSet<T, HashLogic>{&GetDoubleFrameAllocator()} {}

  explicit DoubleFrameHashSet(usize capacity)
      : HashSet<T, HashLogic>{HashSet<T, HashLogic>::WithCapacity(
            &GetDoubleFrameAllocator(), capacity)} {}

  DoubleFrameHashSet(const DoubleFrameHashSet&) = delete;
  DoubleFrameHashSet& operator=(const DoubleFrameHashSet&) = delete;
  DoubleFrameHashSet(DoubleFrameHashSet&&) noexcept = default;
  DoubleFrameHashSet& operator=(DoubleFrameHashSet&&) noexcept = default;
};

template <typename Key, typename Value,
          typename HashLogic = internal::DefaultMapHashLogic<Key, Value>>
class FrameMap : public Map<Key, Value, HashLogic> {
 public:
  static FrameMap WithCapacity(usize capacity) { return FrameMap{capacity}; }

  FrameMap() : Map<Key, Value, HashLogic>{&GetFrameAllocator()} {}

  explicit FrameMap(usize capacity)
      : Map<Key, Value, HashLogic>{Map<Key, Value, HashLogic>::WithCapacity(
            &GetFrameAllocator(), capacity)} {}

  FrameMap(const FrameMap&) = delete;
  FrameMap& operator=(const FrameMap&) = delete;
  FrameMap(FrameMap&&) noexcept = default;
  FrameMap& operator=(FrameMap&&) noexcept = default;
};

template <typename Key, typename Value,
          typename HashLogic = internal::DefaultMapHashLogic<Key, Value>>
class DoubleFrameMap : public Map<Key, Value, HashLogic> {
 public:
  static DoubleFrameMap WithCapacity(usize capacity) {
    return DoubleFrameMap{capacity};
  }

  DoubleFrameMap() : Map<Key, Value, HashLogic>{&GetDoubleFrameAllocator()} {}

  explicit DoubleFrameMap(usize capacity)
      : Map<Key, Value, HashLogic>{Map<Key, Value, HashLogic>::WithCapacity(
            &GetDoubleFrameAllocator(), capacity)} {}

  DoubleFrameMap(const DoubleFrameMap&) = delete;
  DoubleFrameMap& operator=(const DoubleFrameMap&) = delete;
  DoubleFrameMap(DoubleFrameMap&&) noexcept = default;
  DoubleFrameMap& operator=(DoubleFrameMap&&) noexcept = default;
};

template <typename T, typename HashLogic = internal::DefaultSetHashLogic<T>>
class FrameOrderedSet : public OrderedSet<T, HashLogic> {
 public:
  static FrameOrderedSet WithCapacity(usize capacity) {
    return FrameOrderedSet{capacity};
  }

  FrameOrderedSet() : OrderedSet<T, HashLogic>{&GetFrameAllocator()} {}

  explicit FrameOrderedSet(usize capacity)
      : OrderedSet<T, HashLogic>{OrderedSet<T, HashLogic>::WithCapacity(
            &GetFrameAllocator(), capacity)} {}

  FrameOrderedSet(const FrameOrderedSet&) = delete;
  FrameOrderedSet& operator=(const FrameOrderedSet&) = delete;
  FrameOrderedSet(FrameOrderedSet&&) noexcept = default;
  FrameOrderedSet& operator=(FrameOrderedSet&&) noexcept = default;
};

template <typename T, typename HashLogic = internal::DefaultSetHashLogic<T>>
class DoubleFrameOrderedSet : public OrderedSet<T, HashLogic> {
 public:
  static DoubleFrameOrderedSet WithCapacity(usize capacity) {
    return DoubleFrameOrderedSet{capacity};
  }

  DoubleFrameOrderedSet()
      : OrderedSet<T, HashLogic>{&GetDoubleFrameAllocator()} {}

  explicit DoubleFrameOrderedSet(usize capacity)
      : OrderedSet<T, HashLogic>{OrderedSet<T, HashLogic>::WithCapacity(
            &GetDoubleFrameAllocator(), capacity)} {}

  DoubleFrameOrderedSet(const DoubleFrameOrderedSet&) = delete;
  DoubleFrameOrderedSet& operator=(const DoubleFrameOrderedSet&) = delete;
  DoubleFrameOrderedSet(DoubleFrameOrderedSet&&) noexcept = default;
  DoubleFrameOrderedSet& operator=(DoubleFrameOrderedSet&&) noexcept = default;
};

template <typename T>
FrameArray<T>* GenerateFrameArrayWithCapacity(usize capacity) {
  return GetFrameAllocator().AllocateOneAndPopulate<FrameArray<T>>(capacity);
}

template <typename T>
DoubleFrameArray<T>* GenerateDoubleFrameArrayWithCapacity(usize capacity) {
  return GetDoubleFrameAllocator().AllocateOneAndPopulate<DoubleFrameArray<T>>(
      capacity);
}

inline FrameBitset* GenerateFrameBitsetWithSize(usize bit_count) {
  return GetFrameAllocator().AllocateOneAndPopulate<FrameBitset>(bit_count);
}

inline DoubleFrameBitset* GenerateDoubleFrameBitsetWithSize(usize bit_count) {
  return GetDoubleFrameAllocator().AllocateOneAndPopulate<DoubleFrameBitset>(
      bit_count);
}

template <typename T, typename HashLogic = internal::DefaultSetHashLogic<T>>
FrameHashSet<T, HashLogic>* GenerateFrameHashSetWithCapacity(usize capacity) {
  return GetFrameAllocator().AllocateOneAndPopulate<FrameHashSet<T, HashLogic>>(
      capacity);
}

template <typename T, typename HashLogic = internal::DefaultSetHashLogic<T>>
DoubleFrameHashSet<T, HashLogic>* GenerateDoubleFrameHashSetWithCapacity(
    usize capacity) {
  return GetDoubleFrameAllocator()
      .AllocateOneAndPopulate<DoubleFrameHashSet<T, HashLogic>>(capacity);
}

template <typename Key, typename Value,
          typename HashLogic = internal::DefaultMapHashLogic<Key, Value>>
FrameMap<Key, Value, HashLogic>* GenerateFrameMapWithCapacity(usize capacity) {
  return GetFrameAllocator()
      .AllocateOneAndPopulate<FrameMap<Key, Value, HashLogic>>(capacity);
}

template <typename Key, typename Value,
          typename HashLogic = internal::DefaultMapHashLogic<Key, Value>>
DoubleFrameMap<Key, Value, HashLogic>* GenerateDoubleFrameMapWithCapacity(
    usize capacity) {
  return GetDoubleFrameAllocator()
      .AllocateOneAndPopulate<DoubleFrameMap<Key, Value, HashLogic>>(capacity);
}

template <typename T, typename HashLogic = internal::DefaultSetHashLogic<T>>
FrameOrderedSet<T, HashLogic>* GenerateFrameOrderedSetWithCapacity(
    usize capacity) {
  return GetFrameAllocator()
      .AllocateOneAndPopulate<FrameOrderedSet<T, HashLogic>>(capacity);
}

template <typename T, typename HashLogic = internal::DefaultSetHashLogic<T>>
DoubleFrameOrderedSet<T, HashLogic>* GenerateDoubleFrameOrderedSetWithCapacity(
    usize capacity) {
  return GetDoubleFrameAllocator()
      .AllocateOneAndPopulate<DoubleFrameOrderedSet<T, HashLogic>>(capacity);
}
}  // namespace frame
}  // namespace comet

#define COMET_FRAME_ARRAY(T, ...) \
  COMET_FRAME_ALLOC_ONE_AND_POPULATE(comet::frame::FrameArray<T>, ##__VA_ARGS__)

#define COMET_DOUBLE_FRAME_ARRAY(T, ...)                                       \
  COMET_DOUBLE_FRAME_ALLOC_ONE_AND_POPULATE(comet::frame::DoubleFrameArray<T>, \
                                            ##__VA_ARGS__)

#define COMET_FRAME_BITSET(...) \
  COMET_FRAME_ALLOC_ONE_AND_POPULATE(comet::frame::FrameBitset, ##__VA_ARGS__)

#define COMET_DOUBLE_FRAME_BITSET(...)                                       \
  COMET_DOUBLE_FRAME_ALLOC_ONE_AND_POPULATE(comet::frame::DoubleFrameBitset, \
                                            ##__VA_ARGS__)

#define COMET_FRAME_HASH_SET(T, ...)                                \
  COMET_FRAME_ALLOC_ONE_AND_POPULATE(comet::frame::FrameHashSet<T>, \
                                     ##__VA_ARGS__)

#define COMET_DOUBLE_FRAME_HASH_SET(T, ...)  \
  COMET_DOUBLE_FRAME_ALLOC_ONE_AND_POPULATE( \
      comet::frame::DoubleFrameHashSet<T>, ##__VA_ARGS__)

#define COMET_FRAME_MAP(K, V, ...)                                            \
  COMET_FRAME_ALLOC_ONE_AND_POPULATE(comet::frame::FrameMap<K COMET_COMMA V>, \
                                     ##__VA_ARGS__)

#define COMET_DOUBLE_FRAME_MAP(K, V, ...)    \
  COMET_DOUBLE_FRAME_ALLOC_ONE_AND_POPULATE( \
      comet::frame::DoubleFrameMap<K COMET_COMMA V>, ##__VA_ARGS__)

#define COMET_FRAME_ORDERED_SET(T, ...)                                \
  COMET_FRAME_ALLOC_ONE_AND_POPULATE(comet::frame::FrameOrderedSet<T>, \
                                     ##__VA_ARGS__)

#define COMET_DOUBLE_FRAME_ORDERED_SET(T, ...) \
  COMET_DOUBLE_FRAME_ALLOC_ONE_AND_POPULATE(   \
      comet::frame::DoubleFrameOrderedSet<T>, ##__VA_ARGS__)

#define COMET_FRAME_ARRAY_WITH_CAPACITY(T, capacity) \
  comet::frame::GenerateFrameArrayWithCapacity<T>(capacity)

#define COMET_DOUBLE_FRAME_ARRAY_WITH_CAPACITY(T, capacity) \
  comet::frame::GenerateDoubleFrameArrayWithCapacity<T>(capacity)

#define COMET_FRAME_BITSET_WITH_SIZE(bit_count) \
  comet::frame::GenerateFrameBitsetWithSize(bit_count)

#define COMET_DOUBLE_FRAME_BITSET_WITH_SIZE(bit_count) \
  comet::frame::GenerateDoubleFrameBitsetWithSize(bit_count)

#define COMET_FRAME_HASH_SET_WITH_CAPACITY(T, capacity) \
  comet::frame::GenerateFrameHashSetWithCapacity<T>(capacity)

#define COMET_DOUBLE_FRAME_HASH_SET_WITH_CAPACITY(T, capacity) \
  comet::frame::GenerateDoubleFrameHashSetWithCapacity<T>(capacity)

#define COMET_FRAME_MAP_WITH_CAPACITY(K, V, capacity) \
  comet::frame::GenerateFrameMapWithCapacity<K COMET_COMMA V>(capacity)

#define COMET_DOUBLE_FRAME_MAP_WITH_CAPACITY(K, V, capacity) \
  comet::frame::GenerateDoubleFrameMapWithCapacity<K COMET_COMMA V>(capacity)

#define COMET_FRAME_ORDERED_SET_WITH_CAPACITY(T, capacity) \
  comet::frame::GenerateFrameOrderedSetWithCapacity<T>(capacity)

#define COMET_DOUBLE_FRAME_ORDERED_SET_WITH_CAPACITY(T, capacity) \
  comet::frame::GenerateDoubleFrameOrderedSetWithCapacity<T>(capacity)

#endif  // COMET_COMET_CORE_FRAME_FRAME_CONTAINER_H_