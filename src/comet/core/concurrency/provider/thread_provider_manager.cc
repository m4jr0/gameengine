// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "thread_provider_manager.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/memory/tagged_heap.h"

namespace comet {
namespace thread {
ThreadProviderManager& ThreadProviderManager::Get() {
  static ThreadProviderManager singleton{};
  return singleton;
}

ThreadProviderManager::ThreadProviderManager()
    : allocator_{128, 32, kEngineMemoryTag_} {}

void ThreadProviderManager::Initialize() {
  Manager::Initialize();
  allocator_.Initialize();
}

void ThreadProviderManager::Shutdown() {
  Manager::Shutdown();
  allocator_.Destroy();
}
}  // namespace thread
}  // namespace comet
