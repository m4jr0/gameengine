// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_CONCURRENCY_PROVIDER_THREAD_PROVIDER_MANAGER_H_
#define COMET_COMET_CORE_CONCURRENCY_PROVIDER_THREAD_PROVIDER_MANAGER_H_

#include "comet/core/concurrency/provider/thread_provider.h"
#include "comet/core/essentials.h"
#include "comet/core/manager.h"
#include "comet/core/memory/allocator/free_list_allocator.h"
#include "comet/core/memory/memory.h"

namespace comet {
namespace thread {
class ThreadProviderManager : public Manager {
 public:
  static ThreadProviderManager& Get();

  ThreadProviderManager();
  ThreadProviderManager(const ThreadProviderManager&) = delete;
  ThreadProviderManager(ThreadProviderManager&&) noexcept = delete;
  ThreadProviderManager& operator=(const ThreadProviderManager&) = delete;
  ThreadProviderManager& operator=(ThreadProviderManager&&) noexcept = delete;
  ~ThreadProviderManager() override = default;

  void Initialize() override;
  void Shutdown() override;

  template <typename T>
  FiberThreadProvider<T> AllocateFiberProvider() {
    return FiberThreadProvider<T>{&allocator_};
  }

  template <typename T>
  IOThreadProvider<T> AllocateIOProvider() {
    return IOThreadProvider<T>{&allocator_};
  }

 private:
  inline static constexpr memory::MemoryTag kEngineMemoryTag_{
      memory::kEngineMemoryTagThreadProvider};

  memory::FiberFreeListAllocator allocator_{};
};
}  // namespace thread
}  // namespace comet

#endif  // COMET_COMET_CORE_CONCURRENCY_PROVIDER_THREAD_PROVIDER_MANAGER_H_