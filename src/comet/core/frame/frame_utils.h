// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_FRAME_UTILS_H_
#define COMET_COMET_CORE_FRAME_UTILS_H_

#include "comet/core/essentials.h"
#include "comet/core/frame/frame_allocator.h"
#include "comet/core/type/array.h"
#include "comet/core/type/bitset.h"
#include "comet/core/type/hash_set.h"
#include "comet/core/type/map.h"

namespace comet {
namespace frame {
template <typename T>
class FrameArray : public Array<T> {
 public:
  FrameArray() : Array<T>{&GetFrameAllocator()} {}

  FrameArray(usize capacity) : Array<T>{&GetFrameAllocator(), capacity} {}

  template <typename... Targs>
  FrameArray(Targs&&... args)
      : Array<T>{&GetFrameAllocator(), std::forward<Targs>(args)...} {}

  FrameArray(const T* data, usize count)
      : Array<T>{&GetFrameAllocator(), data, count} {}

  template <typename InputIterator>
  FrameArray(InputIterator from, InputIterator to)
      : Array<T>{&GetFrameAllocator(), from, to} {}
};

template <typename T>
class DoubleFrameArray : public Array<T> {
 public:
  DoubleFrameArray() : Array<T>{&GetDoubleFrameAllocator()} {}

  DoubleFrameArray(usize capacity)
      : Array<T>{&GetDoubleFrameAllocator(), capacity} {}

  template <typename... Targs>
  DoubleFrameArray(Targs&&... args)
      : Array<T>{&GetDoubleFrameAllocator(), std::forward<Targs>(args)...} {}

  DoubleFrameArray(const T* data, usize count)
      : Array<T>{&GetDoubleFrameAllocator(), data, count} {}

  template <typename InputIterator>
  DoubleFrameArray(InputIterator from, InputIterator to)
      : Array<T>{&GetDoubleFrameAllocator(), from, to} {}
};

template <typename T, typename HashLogic = internal::DefaultSetHashLogic<T>>
class FrameHashSet : public HashSet<T, HashLogic> {
 public:
  FrameHashSet() : HashSet<T, HashLogic>{&GetFrameAllocator()} {}

  FrameHashSet(usize default_obj_count)
      : HashSet<T, HashLogic>{&GetFrameAllocator(), default_obj_count} {}
};

template <typename T, typename HashLogic = internal::DefaultSetHashLogic<T>>
class DoubleFrameHashSet : public HashSet<T, HashLogic> {
 public:
  DoubleFrameHashSet() : HashSet<T, HashLogic>{&GetDoubleFrameAllocator()} {}

  DoubleFrameHashSet(usize default_obj_count)
      : HashSet<T, HashLogic>{&GetDoubleFrameAllocator(), default_obj_count} {}
};

template <typename Key, typename Value,
          typename HashLogic = internal::DefaultMapHashLogic<Key, Value>>
class FrameMap : public Map<Key, Value, HashLogic> {
 public:
  FrameMap() : Map<Key, Value, HashLogic>{&GetFrameAllocator()} {}

  FrameMap(usize capacity)
      : Map<Key, Value, HashLogic>{&GetFrameAllocator(), capacity} {}
};

template <typename Key, typename Value,
          typename HashLogic = internal::DefaultMapHashLogic<Key, Value>>
class DoubleFrameMap : public Map<Key, Value, HashLogic> {
 public:
  DoubleFrameMap() : Map<Key, Value, HashLogic>{&GetDoubleFrameAllocator()} {}

  DoubleFrameMap(usize capacity)
      : Map<Key, Value, HashLogic>{&GetDoubleFrameAllocator(), capacity} {}
};

class FrameBitset : public Bitset {
 public:
  FrameBitset(usize bit_count) : Bitset{&GetFrameAllocator(), bit_count} {}
};

class DoubleFrameBitset : public Bitset {
 public:
  DoubleFrameBitset(usize bit_count)
      : Bitset{&GetDoubleFrameAllocator(), bit_count} {}
};
}  // namespace frame
}  // namespace comet

#endif  // COMET_COMET_CORE_FRAME_UTILS_H_
