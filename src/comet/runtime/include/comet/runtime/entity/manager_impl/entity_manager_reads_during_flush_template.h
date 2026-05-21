// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_RUNTIME_ENTITY_MANAGER_IMPL_ENTITY_MANAGER_READS_DURING_FLUSH_TEMPLATE_H_
#define COMET_RUNTIME_ENTITY_MANAGER_IMPL_ENTITY_MANAGER_READS_DURING_FLUSH_TEMPLATE_H_

#include "comet/core/essentials.h"

namespace comet {
namespace entity {
static thread_local usize tls_entity_read_depth{0};

template <typename Fn>
decltype(auto) EntityManager::ReadSnapshot(
    [[maybe_unused]] const schar* context, Fn&& fn) const {
  if (tls_entity_read_depth > 0) {
    return std::forward<Fn>(fn)();
  }

  ++tls_entity_read_depth;
  struct ReadDepthGuard {
    ~ReadDepthGuard() { --tls_entity_read_depth; }
  } depth_guard{};

  fiber::FiberSharedLockGuard read_lock{snapshot_mutex_,
                                        fiber::FiberSharedLockType::Shared};

#ifdef COMET_DEBUG
  DiagnosePublicReadDuringFlush(context);
#endif  // COMET_DEBUG

  return std::forward<Fn>(fn)();
}
}  // namespace entity
}  // namespace comet

#endif  // COMET_RUNTIME_ENTITY_MANAGER_IMPL_ENTITY_MANAGER_READS_DURING_FLUSH_TEMPLATE_H_