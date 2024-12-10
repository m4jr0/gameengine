// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_HASH_H_
#define COMET_COMET_CORE_HASH_H_

#include "picosha2.h"

#include "comet/core/essentials.h"
#include "comet/core/memory/memory_utils.h"
#include "comet/math/math_commons.h"

namespace comet {
namespace internal {
constexpr ux kPhi{(1 + math::CtSqrt(ux{5})) / 2};
constexpr ux kMagicNumber{2 ^ kCharBit / kPhi};

template <typename T, typename = std::enable_if_t<std::is_floating_point_v<T>>>
std::size_t Convert(T value) {
  std::size_t to_return;
  memory::CopyMemory(&to_return, &value, sizeof(T));
  return to_return;
}
}  // namespace internal

u32 HashCrC32(const void* data, usize length);

constexpr auto kSha256DigestSize{picosha2::k_digest_size};

void HashSha256(std::ifstream& stream, schar* buffer, usize buffer_len);

constexpr std::size_t HashCombine(std::size_t lhs, std::size_t rhs) {
  lhs ^= rhs + internal::kMagicNumber + (lhs << 6) + (lhs >> 2);
  return lhs;
}

template <typename T, typename = std::enable_if_t<std::is_floating_point_v<T>>>
std::size_t HashCombine(T lhs, std::size_t rhs) {
  return HashCombine(internal::Convert(lhs), rhs);
}

template <typename T, typename = std::enable_if_t<std::is_floating_point_v<T>>>
std::size_t HashCombine(std::size_t lhs, T rhs) {
  return HashCombine(lhs, internal::Convert(rhs));
}

template <typename T1, typename T2,
          typename = std::enable_if_t<std::is_floating_point_v<T1> &&
                                      std::is_floating_point_v<T2>>>
std::size_t HashCombine(T1 lhs, T2 rhs) {
  return HashCombine(internal::Convert(lhs), internal::Convert(rhs));
}

using HashValue = usize;
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

u32 GenerateHash(s32 value);
u32 GenerateHash(u32 value);
u32 GenerateHash(s64 value);
u32 GenerateHash(u64 value);
u32 GenerateHash(f32 value);
u32 GenerateHash(f64 value);
u32 GenerateHash(schar value);
u32 GenerateHash(wchar value);
u32 GenerateHash(const schar* value);
u32 GenerateHash(const wchar* value);
u32 GenerateHash(const void* value);
}  // namespace comet

#endif  // COMET_COMET_CORE_HASH_H_
