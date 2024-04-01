// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "scheduler_utils.h"

namespace comet {
namespace job {
JobDescr GenerateJobDescr(JobPriority priority, fiber::EntryPoint entry_point,
                          fiber::ParamsHandle params_handle,
                          JobStackSize stack_size, Counter* counter) {
  JobDescr descr{};
  descr.stack_size = stack_size;
  descr.priority = priority;
  descr.entry_point = entry_point;
  descr.params_handle = params_handle;
  descr.counter =
      counter == nullptr ? Scheduler::Get().AllocateCounter() : counter;
  return descr;
}

IOJobDescr GenerateIOJobDescr(fiber::EntryPoint entry_point,
                              fiber::ParamsHandle params_handle,
                              Counter* counter) {
  IOJobDescr descr{};
  descr.entry_point = entry_point;
  descr.params_handle = params_handle;
  descr.counter =
      counter == nullptr ? Scheduler::Get().AllocateCounter() : counter;
  return descr;
}
}  // namespace job
}  // namespace comet