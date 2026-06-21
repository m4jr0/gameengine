// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_RUNTIME_CONF_CONF_VALUE_H_
#define COMET_RUNTIME_CONF_CONF_VALUE_H_

#include "comet/core/essentials.h"
#include "comet/core/id/string_id.h"
#include "comet/core/id/string_id_allocator.h"

namespace comet {
namespace conf {
using ConfKey = stringid::StringId;

union ConfValue {
  schar str_value[kMaxStrValueLength];
  wchar wstr_value[kMaxStrValueLength * sizeof(wchar)];
  u8 u8_value;
  u16 u16_value;
  u32 u32_value;
  u64 u64_value;
  s8 s8_value;
  s16 s16_value;
  s32 s32_value;
  s64 s64_value;
  f32 f32_value;
  f64 f64_value;
  usize uindex_value;
  ux ux_value;
  sx sx_value;
  fx fx_value;
  bool bool_value;
};

constexpr auto kMaxKeyLength{64};
constexpr auto kMaxValueLength{256};
constexpr auto kMaxLineLength{512};
constexpr u16 kMaxStrValueLength{260};
}  // namespace conf
}  // namespace comet

#endif  // COMET_RUNTIME_CONF_CONF_VALUE_H_