// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_CONTINUATION_CONTINUATION_MANAGER_H_
#define COMET_COMET_CORE_CONTINUATION_CONTINUATION_MANAGER_H_

// External. ///////////////////////////////////////////////////////////////////
#include <concepts>
#include <type_traits>
#include <utility>
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/concurrency/fiber/fiber_primitive.h"
#include "comet/core/continuation/type/continuation_phase.h"
#include "comet/core/essentials.h"
#include "comet/core/manager.h"
#include "comet/core/memory/allocator/platform_allocator.h"
#include "comet/core/type/array.h"

namespace comet {
template <typename T>
concept Continuation = requires(T continuation) {
  { continuation.Poll() } -> std::same_as<bool>;
};

class ContinuationManager : public Manager {
 public:
  static ContinuationManager& Get();

  ContinuationManager();
  ContinuationManager(const ContinuationManager&) = delete;
  ContinuationManager(ContinuationManager&&) = delete;
  ContinuationManager& operator=(const ContinuationManager&) = delete;
  ContinuationManager& operator=(ContinuationManager&&) = delete;
  ~ContinuationManager() override = default;

  template <Continuation ContinuationType>
  void Submit(ContinuationPhase phase, ContinuationType&& continuation) {
    auto handle{GenerateHandle(std::forward<ContinuationType>(continuation))};

    fiber::FiberLockGuard lock{mutex_};
    auto phase_index{GetPhaseIndex(phase)};

    if (is_polling_[phase_index]) {
      next_continuations_[phase_index].PushLast(handle);
    } else {
      continuations_[phase_index].PushLast(handle);
    }
  }

  void Poll(ContinuationPhase phase);
  void Clear();

 protected:
  void OnInitialize() override;
  void OnShutdown() override;

 private:
  static inline constexpr usize kContinuationPhaseCount_{
      static_cast<usize>(ContinuationPhase::AfterEndFrame) + 1};

  struct ContinuationHandle {
    void* object{nullptr};
    bool (*poll)(void*){nullptr};
    void (*destroy)(memory::Allocator*, void*){nullptr};
  };

  using Continuations = Array<ContinuationHandle>;

  static usize GetPhaseIndex(ContinuationPhase phase);

  template <Continuation ContinuationType>
  ContinuationHandle GenerateHandle(ContinuationType&& continuation) {
    using StoredContinuationType = std::decay_t<ContinuationType>;

    auto* memory{allocator_.AllocateAligned(sizeof(StoredContinuationType),
                                            alignof(StoredContinuationType))};

    COMET_ASSERT(memory != nullptr, "ContinuationManager::GenerateHandle",
                 "failed to allocate continuation", "size",
                 sizeof(StoredContinuationType), "align",
                 alignof(StoredContinuationType));

    auto* object{new (memory) StoredContinuationType{
        std::forward<ContinuationType>(continuation)}};

    ContinuationHandle handle{};
    handle.object = object;

    handle.poll = [](void* ptr) -> bool {
      return static_cast<StoredContinuationType*>(ptr)->Poll();
    };

    handle.destroy = [](memory::Allocator* allocator, void* ptr) {
      auto* continuation{static_cast<StoredContinuationType*>(ptr)};
      continuation->~StoredContinuationType();
      allocator->Deallocate(ptr);
    };

    return handle;
  }

  memory::PlatformAllocator allocator_{memory::kEngineMemoryTagContinuation};
  mutable fiber::FiberMutex mutex_{};
  bool is_polling_[kContinuationPhaseCount_]{false};
  Continuations continuations_[kContinuationPhaseCount_]{};
  Continuations next_continuations_[kContinuationPhaseCount_]{};
};
}  // namespace comet

#endif  // COMET_COMET_CORE_CONTINUATION_CONTINUATION_MANAGER_H_
