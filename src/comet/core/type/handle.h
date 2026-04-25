// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_TYPE_HANDLE_H_
#define COMET_COMET_CORE_TYPE_HANDLE_H_

// External. ///////////////////////////////////////////////////////////////////
#include <cstddef>
#include <functional>
#include <ostream>
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/buffer_formatter.h"
#include "comet/core/c_string.h"
#include "comet/core/essentials.h"
#include "comet/core/hash.h"
#include "comet/core/type/gid.h"

namespace comet {
struct InvalidHandleT {};
inline constexpr InvalidHandleT kInvalidHandle{};

template <typename Tag>
class Handle {
 public:
  constexpr Handle() noexcept = default;
  constexpr Handle(InvalidHandleT) noexcept : value_{gid::kInvalidId} {}
  explicit constexpr Handle(gid::Gid value) noexcept : value_{value} {}

  static constexpr Handle Invalid() noexcept { return Handle{kInvalidHandle}; }

  constexpr bool IsValid() const noexcept { return gid::IsValid(value_); }

  constexpr bool IsInvalid() const noexcept { return !IsValid(); }

  explicit constexpr operator bool() const noexcept { return IsValid(); }

  constexpr void Invalidate() noexcept { value_ = gid::kInvalidId; }

  constexpr gid::Gid GetValue() const noexcept { return value_; }

  constexpr gid::Gid GetIndex() const noexcept { return gid::GetIndex(value_); }

  constexpr gid::Gid GetGeneration() const noexcept {
    return gid::GetGeneration(value_);
  }

  friend constexpr bool operator==(Handle lhs, Handle rhs) noexcept = default;
  friend constexpr bool operator!=(Handle lhs, Handle rhs) noexcept = default;

  friend constexpr bool operator==(Handle handle, InvalidHandleT) noexcept {
    return !handle.IsValid();
  }

  friend constexpr bool operator==(InvalidHandleT, Handle handle) noexcept {
    return !handle.IsValid();
  }

  friend constexpr bool operator!=(Handle handle, InvalidHandleT) noexcept {
    return handle.IsValid();
  }

  friend constexpr bool operator!=(InvalidHandleT, Handle handle) noexcept {
    return handle.IsValid();
  }

 private:
  gid::Gid value_{gid::kInvalidId};
};

template <typename Tag>
class HandlePool {
 public:
  using H = Handle<Tag>;

  HandlePool() = default;
  HandlePool(const HandlePool&) = delete;
  HandlePool(HandlePool&&) noexcept = default;
  HandlePool& operator=(const HandlePool&) = delete;
  HandlePool& operator=(HandlePool&&) noexcept = default;
  ~HandlePool() = default;

  H Generate() noexcept { return H{breed_handler_.Generate()}; }

  bool IsAlive(H handle) const noexcept {
    return handle.IsValid() && breed_handler_.IsAlive(handle.GetValue());
  }

  void Destroy(H handle) noexcept {
    if (!IsAlive(handle)) {
      return;
    }

    breed_handler_.Destroy(handle.GetValue());
  }

  void Destroy() noexcept { breed_handler_.Shutdown(); }

 private:
  gid::BreedHandler breed_handler_{};
};

template <typename Tag>
constexpr bool IsValid(Handle<Tag> handle) noexcept {
  return handle.IsValid();
}

template <typename Tag>
constexpr bool IsInvalid(Handle<Tag> handle) noexcept {
  return handle.IsInvalid();
}

template <typename Tag>
constexpr gid::Gid GetRaw(Handle<Tag> handle) noexcept {
  return handle.GetValue();
}

template <typename Tag>
constexpr gid::Gid GetIndex(Handle<Tag> handle) noexcept {
  return handle.GetIndex();
}

template <typename Tag>
constexpr gid::Gid GetGeneration(Handle<Tag> handle) noexcept {
  return handle.GetGeneration();
}

template <typename Tag>
constexpr HashValue GenerateHash(Handle<Tag> handle) noexcept {
  return GenerateHash(handle.GetValue());
}

template <typename Tag>
struct BufferFormatter<Handle<Tag>> {
  static usize Format(Handle<Tag> handle, schar* buffer, usize buffer_len) {
    if (buffer == nullptr || buffer_len == 0) {
      return 0;
    }

    buffer[0] = '\0';
    usize offset{0};

    const auto append{[&](const schar* str) {
      if (str == nullptr || offset >= buffer_len - 1) {
        return;
      }

      const auto len{GetLength(str)};
      const auto remaining{buffer_len - offset - 1};
      const auto copy_len{len < remaining ? len : remaining};

      if (copy_len == 0) {
        return;
      }

      Copy(buffer, str, copy_len, offset);
      offset += copy_len;
      buffer[offset] = '\0';
    }};

    constexpr usize kNumBufferSize{32};
    schar num_buffer[kNumBufferSize]{'\0'};
    usize num_len{0};

    append("H<");

    ConvertToStr(handle.GetValue(), num_buffer, kNumBufferSize, &num_len);
    append(num_buffer);

    append(" [");

    ConvertToStr(handle.GetIndex(), num_buffer, kNumBufferSize, &num_len);
    append(num_buffer);

    append("#");

    ConvertToStr(handle.GetGeneration(), num_buffer, kNumBufferSize, &num_len);
    append(num_buffer);

    append("]>");

    return offset;
  }
};

template <typename Tag>
std::ostream& operator<<(std::ostream& os, const Handle<Tag>& handle) {
  return os << "H<" << handle.GetValue() << " [" << handle.GetIndex() << "#"
            << handle.GetGeneration() << "]>";
}
}  // namespace comet

namespace std {
template <typename Tag>
struct hash<comet::Handle<Tag>> {
  std::size_t operator()(const comet::Handle<Tag>& handle) const noexcept {
    return static_cast<std::size_t>(comet::GenerateHash(handle));
  }
};
}  // namespace std

#endif  // COMET_COMET_CORE_TYPE_HANDLE_H_