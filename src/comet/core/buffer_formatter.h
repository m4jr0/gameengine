// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_BUFFER_FORMATTER_H_
#define COMET_COMET_CORE_BUFFER_FORMATTER_H_

// External. ///////////////////////////////////////////////////////////////////
#include <concepts>
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/debug_label.h"
#include "comet/core/essentials.h"
#include "comet/core/memory/memory_utils.h"

namespace comet {
template <typename T>
struct BufferFormatter;

template <typename T>
concept HasBufferFormatter =
    requires(const T& value, schar* buffer, usize buffer_len) {
      {
        BufferFormatter<T>::Format(value, buffer, buffer_len)
      } -> std::same_as<usize>;
    };

template <typename T>
struct BufferFormatter<T*> {
  static usize Format(const T* value, schar* buffer, usize buffer_len) {
    if (value == nullptr) {
      Copy(buffer, kNullLabel, kNullLabelLen);
      return kNullLabelLen;
    }

    memory::ConvertAddressToHex(static_cast<const void*>(value), buffer,
                                buffer_len);

    return GetLength(buffer);
  }
};
}  // namespace comet

#endif  // COMET_COMET_CORE_BUFFER_FORMATTER_H_