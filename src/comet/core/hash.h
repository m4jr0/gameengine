// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_HASH_H_
#define COMET_COMET_CORE_HASH_H_

// External. ///////////////////////////////////////////////////////////////////
#include "picosha2.h"
////////////////////////////////////////////////////////////////////////////////

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

using HashValue32 = u32;
using HashValue64 = u64;

using HashValue = HashValue64;

constexpr HashValue32 kInvalidHashValue32{0};
constexpr HashValue64 kInvalidHashValue64{0};
constexpr HashValue kInvalidHashValue{kInvalidHashValue64};

constexpr HashValue32 NarrowHash(HashValue64 value) {
  return static_cast<HashValue32>(value) ^
         static_cast<HashValue32>(value >> 32);
}

constexpr HashValue64 PromoteHash(HashValue32 value) {
  return static_cast<HashValue64>(value);
}

HashValue32 HashCrC32(const void* data, usize length);

constexpr auto kSha256DigestSize{picosha2::k_digest_size};

void HashSha256(std::ifstream& stream, schar* buffer, usize buffer_len);

constexpr HashValue64 HashCombine(HashValue64 lhs, HashValue64 rhs) {
  lhs ^= rhs + internal::kU64MagicNumber + (lhs << 6) + (lhs >> 2);
  return lhs;
}

constexpr HashValue32 HashCombine(HashValue32 lhs, HashValue32 rhs) {
  lhs ^= rhs + internal::kU32MagicNumber + (lhs << 6) + (lhs >> 2);
  return lhs;
}

template <typename T, typename = std::enable_if_t<std::is_floating_point_v<T>>>
HashValue64 HashCombine(T lhs, HashValue64 rhs) {
  return HashCombine(internal::Convert(lhs), rhs);
}

template <typename T, typename = std::enable_if_t<std::is_floating_point_v<T>>>
HashValue64 HashCombine(HashValue64 lhs, T rhs) {
  return HashCombine(lhs, internal::Convert(rhs));
}

template <typename T1, typename T2,
          typename = std::enable_if_t<std::is_floating_point_v<T1> &&
                                      std::is_floating_point_v<T2>>>
HashValue64 HashCombine(T1 lhs, T2 rhs) {
  return HashCombine(internal::Convert(lhs), internal::Convert(rhs));
}

template <typename T, typename = std::enable_if_t<std::is_floating_point_v<T>>>
HashValue32 HashCombine(T lhs, HashValue32 rhs) {
  return HashCombine(static_cast<HashValue32>(internal::Convert(lhs)), rhs);
}

template <typename T, typename = std::enable_if_t<std::is_floating_point_v<T>>>
HashValue32 HashCombine(HashValue32 lhs, T rhs) {
  return HashCombine(lhs, static_cast<HashValue32>(internal::Convert(rhs)));
}

template <typename Key>
struct Hash {
  HashValue operator()(const Key& key) const {
    if constexpr (std::is_integral_v<Key> || std::is_enum_v<Key>) {
      return static_cast<HashValue>(key);
    }

    if constexpr (std::is_pointer_v<Key>) {
      return static_cast<HashValue>(reinterpret_cast<uptr>(key));
    }

    HashValue hash{0};
    const auto* ptr{reinterpret_cast<const schar*>(&key)};

    for (usize i{0}; i < sizeof(Key); ++i) {
      hash = hash * 31 + static_cast<u8>(ptr[i]);
    }

    return hash;
  }
};

constexpr HashValue32 kFnvPrime32{16777619};
constexpr HashValue32 kFnvOffsetBasis32{2166136261};
constexpr HashValue64 kFnvPrime64{1099511628211ULL};
constexpr HashValue64 kFnvOffsetBasis64{14695981039346656037ULL};

HashValue32 GenerateHash32(bool value);
HashValue32 GenerateHash32(s8 value);
HashValue32 GenerateHash32(u8 value);
HashValue32 GenerateHash32(s16 value);
HashValue32 GenerateHash32(u16 value);
HashValue32 GenerateHash32(s32 value);
HashValue32 GenerateHash32(u32 value);
HashValue32 GenerateHash32(s64 value);
HashValue32 GenerateHash32(u64 value);
HashValue32 GenerateHash32(f32 value);
HashValue32 GenerateHash32(f64 value);
HashValue32 GenerateHash32(schar value);
HashValue32 GenerateHash32(wchar value);
HashValue32 GenerateHash32(const schar* value);
HashValue32 GenerateHash32(const wchar* value);
HashValue32 GenerateHash32(const void* value);

HashValue64 GenerateHash64(bool value);
HashValue64 GenerateHash64(s8 value);
HashValue64 GenerateHash64(u8 value);
HashValue64 GenerateHash64(s16 value);
HashValue64 GenerateHash64(u16 value);
HashValue64 GenerateHash64(s32 value);
HashValue64 GenerateHash64(u32 value);
HashValue64 GenerateHash64(s64 value);
HashValue64 GenerateHash64(u64 value);
HashValue64 GenerateHash64(f32 value);
HashValue64 GenerateHash64(f64 value);
HashValue64 GenerateHash64(schar value);
HashValue64 GenerateHash64(wchar value);
HashValue64 GenerateHash64(const schar* value);
HashValue64 GenerateHash64(const wchar* value);
HashValue64 GenerateHash64(const void* value);

HashValue GenerateHash(bool value);
HashValue GenerateHash(s8 value);
HashValue GenerateHash(u8 value);
HashValue GenerateHash(s16 value);
HashValue GenerateHash(u16 value);
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
HashValue GenerateHash(const schar* value, usize length);
HashValue GenerateHash(const wchar* value, usize length);
HashValue GenerateHash(const void* value);
}  // namespace comet

#endif  // COMET_COMET_CORE_HASH_H_
