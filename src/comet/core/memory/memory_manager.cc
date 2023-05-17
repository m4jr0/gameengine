// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "memory_manager.h"

#include "allocation_tracking.h"

namespace comet {
namespace memory {
MemoryManager::MemoryManager(const MemoryManagerDescr& descr)
    : Manager{descr} {}

void MemoryManager::Initialize() { Manager::Initialize(); }

void MemoryManager::Shutdown() { Manager::Shutdown(); }

void MemoryManager::Update() {}

uindex MemoryManager::GetAllocatedMemory() const { return GetMemoryUse(); }
}  // namespace memory
}  // namespace comet
