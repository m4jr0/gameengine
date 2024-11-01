// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_TYPE_STL_TYPES_H_
#define COMET_COMET_CORE_TYPE_STL_TYPES_H_

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "comet/core/essentials.h"
#include "comet/core/frame/stl/one_frame_allocator.h"
#include "comet/core/frame/stl/two_frame_allocator.h"

namespace comet {
template <typename T>
using custom_unique_ptr = std::unique_ptr<T, std::function<void(T*)>>;

// One-frame types.
using one_frame_string =
    std::basic_string<char, std::char_traits<char>,
                      frame::one_cycle_frame_allocator<char>>;

template <typename T>
using one_frame_vector = std::vector<T, frame::one_cycle_frame_allocator<T>>;

template <typename K, typename V>
using one_frame_unordered_map =
    std::unordered_map<K, V, std::hash<K>, std::equal_to<K>,
                       frame::one_cycle_frame_allocator<std::pair<const K, V>>>;

// Two-frame types.
using two_frame_string =
    std::basic_string<char, std::char_traits<char>,
                      frame::two_cycle_frame_allocator<char>>;

template <class T>
using two_frame_vector = std::vector<T, frame::two_cycle_frame_allocator<T>>;

template <class K, class V>
using two_frame_unordered_map =
    std::unordered_map<K, V, std::hash<K>, std::equal_to<K>,
                       frame::two_cycle_frame_allocator<std::pair<const K, V>>>;

template <class T, std::size_t size>
constexpr usize GetLength(const T (&)[size]) noexcept {
  return size;
}
}  // namespace comet

#endif  // COMET_COMET_CORE_TYPE_STL_TYPES_H_
