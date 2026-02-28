// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "resource_handler_utils.h"
////////////////////////////////////////////////////////////////////////////////

// External. ///////////////////////////////////////////////////////////////////
#include <type_traits>
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/c_string.h"

namespace comet {
namespace resource {
namespace internal {
bool ResourceIdLifeSpanPair::operator==(
    const ResourceIdLifeSpanPair& other) const {
  return id == other.id && life_span == other.life_span;
}

HashValue GenerateHash(const ResourceIdLifeSpanPair& value) {
  return HashCombine(
      comet::GenerateHash(value.id),
      comet::GenerateHash(static_cast<std::underlying_type_t<ResourceLifeSpan>>(
          value.life_span)));
}

static thread_local TString tls_resource_abs_path_cached{};

TString& GenerateTlsResourceAbsPath(CTStringView root_resource_path,
                                    ResourceId resource_id) {
  constexpr auto kResourceIdPathBufferLen{GetCharCount<ResourceId>() + 1};
  tchar resource_id_path[kResourceIdPathBufferLen];
  usize resource_id_path_len;
  ConvertToStr(resource_id, resource_id_path, kResourceIdPathBufferLen,
               &resource_id_path_len);
  COMET_ASSERT(resource_id_path_len < kResourceIdPathBufferLen,
               "Invalid resource ID: ", resource_id, "!");
  tls_resource_abs_path_cached.Clear();
  tls_resource_abs_path_cached = root_resource_path;
  tls_resource_abs_path_cached /= resource_id_path;
  return tls_resource_abs_path_cached;
}
}  // namespace internal
}  // namespace resource
}  // namespace comet
