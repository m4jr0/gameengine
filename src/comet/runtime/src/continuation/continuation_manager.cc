// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_runtime_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "comet/runtime/continuation/continuation_manager.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/runtime/continuation/continuation_phase_label.h"
#include "comet/runtime/frame/frame_container.h"
#include "comet/runtime/profiler/profiler.h"

namespace comet {
ContinuationManager& ContinuationManager::Get() {
  static ContinuationManager singleton{};
  return singleton;
}

ContinuationManager::ContinuationManager() = default;

void ContinuationManager::OnInitialize() {
  for (auto& continuations : continuations_) {
    continuations = Continuations{&allocator_};
  }

  for (auto& continuations : next_continuations_) {
    continuations = Continuations{&allocator_};
  }
}

void ContinuationManager::OnShutdown() {
  Clear();

  for (auto& continuations : continuations_) {
    continuations.Release();
  }

  for (auto& continuations : next_continuations_) {
    continuations.Release();
  }
}

void ContinuationManager::Poll(ContinuationPhase phase) {
  COMET_PROFILE("ContinuationManager::Poll");
  const auto phase_index{GetPhaseIndex(phase)};

  frame::FrameArray<ContinuationHandle> polling{};
  frame::FrameArray<ContinuationHandle> pending{};
  frame::FrameArray<ContinuationHandle> completed{};

  {
    fiber::FiberLockGuard lock{mutex_};
    auto& continuations{continuations_[phase_index]};

    is_polling_[phase_index] = true;

    for (const auto& continuation : continuations) {
      polling.PushLast(continuation);
    }

    continuations.Clear();
  }

  for (auto& continuation : polling) {
    COMET_ASSERT(continuation.object != nullptr, "ContinuationManager::Poll",
                 "continuation object is null", "phase",
                 GetContinuationPhaseLabel(phase));
    COMET_ASSERT(continuation.poll != nullptr, "ContinuationManager::Poll",
                 "continuation poll function is null", "phase",
                 GetContinuationPhaseLabel(phase));

    if (continuation.poll(continuation.object)) {
      completed.PushLast(continuation);
    } else {
      pending.PushLast(continuation);
    }
  }

  for (auto& continuation : completed) {
    COMET_ASSERT(continuation.object != nullptr, "ContinuationManager::Poll",
                 "continuation object is null", "phase",
                 GetContinuationPhaseLabel(phase));
    COMET_ASSERT(continuation.destroy != nullptr, "ContinuationManager::Poll",
                 "continuation destroy function is null", "phase",
                 GetContinuationPhaseLabel(phase));

    continuation.destroy(&allocator_, continuation.object);
  }

  {
    fiber::FiberLockGuard lock{mutex_};

    auto& continuations{continuations_[phase_index]};
    auto& next_continuations{next_continuations_[phase_index]};

    continuations.Reserve(pending.GetSize() + next_continuations.GetSize());

    for (const auto& continuation : pending) {
      continuations.PushLast(continuation);
    }

    for (const auto& continuation : next_continuations) {
      continuations.PushLast(continuation);
    }

    next_continuations.Clear();
    is_polling_[phase_index] = false;
  }
}

void ContinuationManager::Clear() {
  fiber::FiberLockGuard lock{mutex_};

  for (usize phase_index{0}; phase_index < kContinuationPhaseCount_;
       ++phase_index) {
    {
      auto& continuations{continuations_[phase_index]};

      for (auto& continuation : continuations) {
        if (continuation.destroy != nullptr) {
          COMET_ASSERT(
              continuation.object != nullptr, "ContinuationManager::Clear",
              "continuation object is null", "phase_index", phase_index);
          continuation.destroy(&allocator_, continuation.object);
        }
      }

      continuations.Clear();
    }

    {
      auto& continuations{next_continuations_[phase_index]};

      for (auto& continuation : continuations) {
        if (continuation.destroy != nullptr) {
          COMET_ASSERT(
              continuation.object != nullptr, "ContinuationManager::Clear",
              "continuation object is null", "phase_index", phase_index);
          continuation.destroy(&allocator_, continuation.object);
        }
      }

      continuations.Clear();
    }

    is_polling_[phase_index] = false;
  }
}

usize ContinuationManager::GetPhaseIndex(ContinuationPhase phase) {
  const auto index{static_cast<usize>(phase)};

  COMET_ASSERT(index < kContinuationPhaseCount_,
               "ContinuationManager::GetPhaseIndex",
               "continuation phase is out of bounds", "phase",
               GetContinuationPhaseLabel(phase), "phase_value", index);

  return index;
}
}  // namespace comet
