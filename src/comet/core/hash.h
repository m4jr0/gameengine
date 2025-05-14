// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_HASH_H_
#define COMET_COMET_CORE_HASH_H_

#include "picosha2.h"

#include "comet/core/essentials.h"
#include "comet/core/memory/memory_utils.h"

namespace comet {
namespace internal {
constexpr u32 kU32MagicNumber{0x9e3779b9};
constexpr u64 kU64MagicNumber{0x9e3779b97f4a7c15};

template <typename T, typename = std::enable_if_t<std::is_floating_point_v<T>>>
u64 Convert(T value) {
  u64 to_return;
  memory::CopyMemory(&to_return, &value, sizeof(T));
  return to_return;
}
}  // namespace internal

u32 HashCrC32(const void* data, usize length);

constexpr auto kSha256DigestSize{picosha2::k_digest_size};

void HashSha256(std::ifstream& stream, schar* buffer, usize buffer_len);

constexpr u64 HashCombine(u64 lhs, u64 rhs) {
  lhs ^= rhs + internal::kU64MagicNumber + (lhs << 6) + (lhs >> 2);
  return lhs;
}

constexpr u32 HashCombine(u32 lhs, u32 rhs) {
  lhs ^= rhs + internal::kU32MagicNumber + (lhs << 6) + (lhs >> 2);
  return lhs;
}

template <typename T, typename = std::enable_if_t<std::is_floating_point_v<T>>>
u64 HashCombine(T lhs, u64 rhs) {
  return HashCombine(internal::Convert(lhs), rhs);
}

template <typename T, typename = std::enable_if_t<std::is_floating_point_v<T>>>
u64 HashCombine(u64 lhs, T rhs) {
  return HashCombine(lhs, internal::Convert(rhs));
}

template <typename T1, typename T2,
          typename = std::enable_if_t<std::is_floating_point_v<T1> &&
                                      std::is_floating_point_v<T2>>>
u64 HashCombine(T1 lhs, T2 rhs) {
  return HashCombine(internal::Convert(lhs), internal::Convert(rhs));
}

template <typename T, typename = std::enable_if_t<std::is_floating_point_v<T>>>
u32 HashCombine(T lhs, u32 rhs) {
  return HashCombine(static_cast<u32>(internal::Convert(lhs)), rhs);
}

template <typename T, typename = std::enable_if_t<std::is_floating_point_v<T>>>
u32 HashCombine(u32 lhs, T rhs) {
  return HashCombine(lhs, static_cast<u32>(internal::Convert(rhs)));
}

using HashValue = u32;
constexpr HashValue kInvalidHashValue{0};

template <typename Key>
struct Hash {
  HashValue operator()(const Key& key) const {
    if constexpr (std::is_integral_v<Key> || std::is_enum_v<Key>) {
      return static_cast<HashValue>(key);
    }

    if constexpr (std::is_pointer_v<Key>) {
      return reinterpret_cast<HashValue>(key);
    }

    HashValue hash{0};
    const auto* ptr{reinterpret_cast<const schar*>(&key)};

    for (usize i{0}; i < sizeof(Key); ++i) {
      hash = hash * 31 + static_cast<u8>(ptr[i]);
    }

    return hash;
  }
};

constexpr u32 kFnvPrime32{16777619};
constexpr u32 kFnvOffsetBasis32{2166136261};
constexpr u64 kFnvPrime64{1099511628211ULL};
constexpr u64 kFnvOffsetBasis64{14695981039346656037ULL};

HashValue GenerateHash(s32 value);
HashValue GenerateHash(u32 value);
HashValue GenerateHash(s64 value);
HashValue GenerateHash(u64 value);
HashValue GenerateHash(f32 value);
HashValue GenerateHash(f64 value);
HashValue GenerateHash(schar value);
HashValue GenerateHash(wchar value);
HashValue GenerateHash(const schar* value);
HashValue GenerateHash(const wchar* value);
HashValue GenerateHash(const void* value);
}  // namespace comet

#endif  // COMET_COMET_CORE_HASH_H_
