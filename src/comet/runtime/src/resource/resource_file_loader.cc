// runtime/resource/resource_file_loader.cc

// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "comet/runtime/resource/resource_file_loader.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/job/job.h"
#include "comet/core/job/job_utils.h"
#include "comet/core/job/scheduler.h"

namespace comet {
namespace resource {
bool LoadResourceFile(CTStringView path, ResourceFile& file) {
  COMET_ASSERT(!path.IsEmpty(), "resource::LoadResourceFile", "path is empty");

  struct JobParams {
    bool is_loaded{false};
    const tchar* path{nullptr};
    ResourceFile* file{nullptr};
  };

  JobParams params{};
  params.path = path.GetCTStr();
  params.file = &file;

  job::CounterGuard guard{};

  job::Scheduler::Get().KickAndWait(job::GenerateIOJobDescr(
      [](job::IOJobParamsHandle params_handle) {
        auto* params{reinterpret_cast<JobParams*>(params_handle)};
        COMET_ASSERT(params != nullptr, "resource::LoadResourceFile",
                     "job params are null");
        COMET_ASSERT(params->path != nullptr, "resource::LoadResourceFile",
                     "path is null");
        COMET_ASSERT(params->file != nullptr, "resource::LoadResourceFile",
                     "resource file output is null");

        params->is_loaded = ReadResourceFile(params->path, *params->file);
      },
      &params, guard.GetCounter()));

  return params.is_loaded;
}
}  // namespace resource
}  // namespace comet