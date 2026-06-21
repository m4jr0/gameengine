// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_DATA_RESOURCE_RESOURCE_ID_H_
#define COMET_DATA_RESOURCE_RESOURCE_ID_H_

// External. ///////////////////////////////////////////////////////////////////
#include <cstddef>
#include <functional>
#include <ostream>
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/essentials.h"
#include "comet/core/format/buffer_formatter.h"
#include "comet/core/hash/hash.h"
#include "comet/data/resource/common.h"

namespace comet {
namespace resource {
struct InvalidResourceIdT {};
inline constexpr InvalidResourceIdT kInvalidResourceIdT{};

template <typename Tag>
class ResourceIdT {
 public:
  constexpr ResourceIdT() noexcept = default;
  constexpr ResourceIdT(InvalidResourceIdT) noexcept
      : value_{kInvalidRawResourceId} {}
  explicit constexpr ResourceIdT(RawResourceId value) noexcept
      : value_{value} {}

  static constexpr ResourceIdT Invalid() noexcept {
    return ResourceIdT{kInvalidResourceIdT};
  }

  constexpr bool IsValid() const noexcept {
    return value_ != kInvalidRawResourceId;
  }

  constexpr bool IsInvalid() const noexcept { return !IsValid(); }

  explicit constexpr operator bool() const noexcept { return IsValid(); }

  constexpr void Invalidate() noexcept { value_ = kInvalidRawResourceId; }

  constexpr RawResourceId GetValue() const noexcept { return value_; }

  friend constexpr bool operator==(ResourceIdT lhs,
                                   ResourceIdT rhs) noexcept = default;
  friend constexpr bool operator!=(ResourceIdT lhs,
                                   ResourceIdT rhs) noexcept = default;

  friend constexpr bool operator==(ResourceIdT id,
                                   InvalidResourceIdT) noexcept {
    return !id.IsValid();
  }

  friend constexpr bool operator==(InvalidResourceIdT,
                                   ResourceIdT id) noexcept {
    return !id.IsValid();
  }

  friend constexpr bool operator!=(ResourceIdT id,
                                   InvalidResourceIdT) noexcept {
    return id.IsValid();
  }

  friend constexpr bool operator!=(InvalidResourceIdT,
                                   ResourceIdT id) noexcept {
    return id.IsValid();
  }

 private:
  RawResourceId value_{kInvalidRawResourceId};
};

template <typename Tag>
constexpr bool IsValid(ResourceIdT<Tag> id) noexcept {
  return id.IsValid();
}

template <typename Tag>
constexpr bool IsInvalid(ResourceIdT<Tag> id) noexcept {
  return id.IsInvalid();
}

template <typename Tag>
constexpr RawResourceId GetRaw(ResourceIdT<Tag> id) noexcept {
  return id.GetValue();
}

template <typename Tag>
constexpr HashValue GenerateHash(ResourceIdT<Tag> id) noexcept {
  return comet::GenerateHash(id.GetValue());
}

template <typename Tag>
std::ostream& operator<<(std::ostream& os, const ResourceIdT<Tag>& id) {
  return os << "R<" << id.GetValue() << ">";
}
}  // namespace resource

template <typename Tag>
struct BufferFormatter<resource::ResourceIdT<Tag>> {
  static usize Format(resource::ResourceIdT<Tag> id, schar* buffer,
                      usize buffer_len) {
    if (buffer == nullptr || buffer_len == 0) {
      return 0;
    }

    buffer[0] = '\0';
    usize offset{0};

    const auto append = [&](const schar* str) {
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
    };

    constexpr usize kNumBufferSize{32};
    schar num_buffer[kNumBufferSize]{'\0'};
    usize num_len{0};

    append("R<");
    ConvertToStr(id.GetValue(), num_buffer, kNumBufferSize, &num_len);
    append(num_buffer);
    append(">");

    return offset;
  }
};
}  // namespace comet

namespace std {
template <typename Tag>
struct hash<comet::resource::ResourceIdT<Tag>> {
  std::size_t operator()(
      const comet::resource::ResourceIdT<Tag>& id) const noexcept {
    return static_cast<std::size_t>(comet::resource::GenerateHash(id));
  }
};
}  // namespace std

#endif  // COMET_DATA_RESOURCE_RESOURCE_ID_H_