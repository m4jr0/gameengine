// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_MEMORY_MEMORY_MANAGER_H_
#define COMET_COMET_CORE_MEMORY_MEMORY_MANAGER_H_

#include "comet_precompile.h"

#include "comet/core/manager.h"

namespace comet {
namespace memory {
using MemoryManagerDescr = ManagerDescr;

class MemoryManager : public Manager {
 public:
  MemoryManager() = delete;
  explicit MemoryManager(const MemoryManagerDescr& descr);
  MemoryManager(const MemoryManager&) = delete;
  MemoryManager(MemoryManager&&) = delete;
  MemoryManager& operator=(const MemoryManager&) = delete;
  MemoryManager& operator=(MemoryManager&&) = delete;
  virtual ~MemoryManager() = default;

  void Initialize() override;
  void Shutdown() override;
  void Update();

  uindex GetAllocatedMemory() const;
};
}  // namespace memory
}  // namespace comet

#endif  // COMET_COMET_CORE_MEMORY_MEMORY_MANAGER_H_
