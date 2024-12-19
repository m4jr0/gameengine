// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "profiler_data.h"

#ifdef COMET_PROFILING
namespace comet {
namespace profiler {
ProfilerData::ProfilerData(memory::Allocator* allocator)
    : record_context{allocator} {}
}  // namespace profiler
}  // namespace comet
#endif  // COMET_PROFILING
