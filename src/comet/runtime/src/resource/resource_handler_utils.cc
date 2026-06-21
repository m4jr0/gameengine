// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_runtime_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "comet/runtime/resource/resource_handler_utils.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/string/c_string.h"
#include "comet/core/type_trait.h"

namespace comet {
namespace resource {
namespace internal {
bool ResourceIdLifeSpanPair::operator==(
    const ResourceIdLifeSpanPair& other) const {
  return id == other.id && life_span == other.life_span;
}

HashValue GenerateHash(const ResourceIdLifeSpanPair& value) {
  return HashCombine(comet::GenerateHash(value.id),
                     comet::GenerateHash(ToUnderlying(value.life_span)));
}

static thread_local TString tls_resource_abs_path_cached{};

TString& GenerateTlsResourceAbsPath(CTStringView root_resource_path,
                                    RawResourceId resource_id) {
  COMET_ASSERT(!root_resource_path.IsEmpty(),
               "resource_handler_utils::internal::GenerateTlsResourceAbsPath",
               "root resource path is empty");
  COMET_ASSERT(resource_id != kInvalidRawResourceId,
               "resource_handler_utils::internal::GenerateTlsResourceAbsPath",
               "resource id is invalid");

  constexpr auto kResourceIdPathBufferLen{GetCharCount<RawResourceId>() + 1};
  tchar resource_id_path[kResourceIdPathBufferLen];
  usize resource_id_path_len;

  ConvertToStr(resource_id, resource_id_path, kResourceIdPathBufferLen,
               &resource_id_path_len);
  COMET_ASSERT(resource_id_path_len < kResourceIdPathBufferLen,
               "resource_handler_utils::internal::GenerateTlsResourceAbsPath",
               "resource id string conversion overflowed", "resource_id",
               resource_id, "buffer_length", kResourceIdPathBufferLen);

  tls_resource_abs_path_cached.Clear();
  tls_resource_abs_path_cached = root_resource_path;
  tls_resource_abs_path_cached /= resource_id_path;
  COMET_ASSERT(!tls_resource_abs_path_cached.IsEmpty(),
               "resource_handler_utils::internal::GenerateTlsResourceAbsPath",
               "generated resource absolute path is empty", "resource_id",
               resource_id);

  return tls_resource_abs_path_cached;
}
}  // namespace internal
}  // namespace resource
}  // namespace comet
