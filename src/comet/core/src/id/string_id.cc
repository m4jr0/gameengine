// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_core_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "comet/core/id/string_id.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/string/c_string.h"
#include "comet/core/hash/hash.h"
#include "comet/core/memory/memory.h"
#include "comet/runtime/memory/memory_tag.h"
#include "comet/runtime/memory/tagged_memory.h"
#include "comet/core/id/string_id_allocator.h"

#ifdef COMET_LABELIZE_STRING_IDS
// External. ///////////////////////////////////////////////////////////////////
#include <mutex>
#include <shared_mutex>
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/type/string_id_allocator.h"

#if defined(COMET_DEBUG_STRING_ID_ALLOCATOR) && \
    defined(COMET_VERBOSE_ALLOCATOR_LOGS)
#include "comet/core/logger/logging.h"
#endif  // defined(COMET_DEBUG_STRING_ID_ALLOCATOR) &&
        //  defined(COMET_VERBOSE_ALLOCATOR_LOGS)
#endif  // COMET_LABELIZE_STRING_IDS

namespace comet {
namespace stringid {
StringIdHandler::~StringIdHandler() {
#ifdef COMET_LABELIZE_STRING_IDS
  auto& debug_data{internal::GetDebugData()};
  std::unique_lock<std::shared_mutex> lock{debug_data.label_mutex};
  debug_data.Destroy();
#endif  // COMET_LABELIZE_STRING_IDS
}

StringId StringIdHandler::Generate(const schar* str, usize length) {
  COMET_ASSERT(str != nullptr, "StringIdHandler::Generate", "string is null");
  COMET_ASSERT(length > 0, "StringIdHandler::Generate",
               "string length is zero");
  const auto string_id{HashCrC32(str, length)};

#ifdef COMET_LABELIZE_STRING_IDS
  auto& debug_data{internal::GetDebugData()};
  std::unique_lock<std::shared_mutex> lock{debug_data.label_mutex};
  debug_data.InitializeIfNeeded();

  if (!debug_data.label_table.IsContained(string_id)) {
    auto* saved_str{
        debug_data.string_id_allocator.AllocateMany<schar>(length + 1)};
    Copy(saved_str, str, length);
    saved_str[length] = '\0';
    debug_data.label_table.Emplace(string_id, saved_str);
  }
#endif  // COMET_LABELIZE_STRING_IDS

  return string_id;
}

StringId StringIdHandler::Generate(const wchar* str, usize length) {
  COMET_ASSERT(str != nullptr, "StringIdHandler::Generate", "string is null");
  COMET_ASSERT(length > 0, "StringIdHandler::Generate",
               "string length is zero");

  constexpr usize kMaxTmpLength{1024};
  schar tmp[kMaxTmpLength]{'\0'};
  const auto tmp_len{length < kMaxTmpLength - 1 ? length : kMaxTmpLength - 1};

  Copy(tmp, str, tmp_len);
  tmp[tmp_len] = '\0';

  return Generate(tmp, tmp_len);
}

StringId StringIdHandler::Generate(const schar* str) {
  return Generate(str, GetLength(str));
}

StringId StringIdHandler::Generate(const wchar* str) {
  return Generate(str, GetLength(str));
}

// Return temporary string for debug purposes. The schar* returned SHOULD NOT be
// stored.
const schar* StringIdHandler::Labelize(StringId string_id) const {
#ifdef COMET_LABELIZE_STRING_IDS
  auto& debug_data{internal::GetDebugData()};
  std::shared_lock<std::shared_mutex> lock{debug_data.label_mutex};
  debug_data.InitializeIfNeeded();
  const auto* label{debug_data.label_table.TryGet(string_id)};

  if (label == nullptr) {
#endif  // COMET_LABELIZE_STRING_IDS
    thread_local schar placeholder[16]{'\0'};
    usize len{0};

    placeholder[0] = '?';
    ConvertToStr(string_id, placeholder + 1, 14, &len);
    placeholder[len + 1] = '?';
    placeholder[len + 2] = '\0';

    return placeholder;
#ifdef COMET_LABELIZE_STRING_IDS
  }

  return *label;
#endif  // COMET_LABELIZE_STRING_IDS
}

StringIdHandler* SetHandler(bool is_destroy) {
  static memory::UniquePtr<stringid::StringIdHandler> string_id_handler{
      nullptr};

  if (is_destroy) {
    string_id_handler = nullptr;
    return string_id_handler.get();
  }

  if (string_id_handler == nullptr) {
    string_id_handler = std::make_unique<stringid::StringIdHandler>();
  }

  return string_id_handler.get();
}
}  // namespace stringid
}  // namespace comet
