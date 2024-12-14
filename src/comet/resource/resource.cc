// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "resource.h"

#include <fstream>

#include "comet/core/compression.h"
#include "comet/core/concurrency/job/job.h"
#include "comet/core/concurrency/job/job_utils.h"
#include "comet/core/concurrency/job/scheduler.h"
#include "comet/core/file_system/file_system.h"
#include "comet/core/generator.h"
#include "comet/core/logger.h"
#include "comet/core/memory/memory_utils.h"
#include "comet/math/math_commons.h"
#include "comet/profiler/profiler.h"

namespace comet {
namespace resource {
ResourceId GenerateResourceIdFromPath(CTStringView resource_path) {
  return COMET_STRING_ID(resource_path);
}

void PackBytes(const u8* bytes, usize bytes_size,
               CompressionMode compression_mode, Array<u8>* packed_bytes,
               usize* packed_bytes_size) {
  switch (compression_mode) {
    case CompressionMode::Lz4: {
      CompressLz4(bytes, bytes_size, *packed_bytes);
      *packed_bytes_size = packed_bytes->GetSize();
      break;
    }
    case CompressionMode::None: {
      memory::CopyMemory(packed_bytes, bytes, bytes_size);
      *packed_bytes_size = bytes_size;
      break;
    }
    default: {
      throw std::runtime_error(
          "Unknown compression mode: " +
          static_cast<std::underlying_type_t<resource::CompressionMode>>(
              compression_mode));
    }
  }
}

void PackBytes(const Array<u8>& bytes, CompressionMode compression_mode,
               Array<u8>* packed_bytes, usize* packed_bytes_size) {
  PackBytes(bytes.GetData(), bytes.GetSize(), compression_mode, packed_bytes,
            packed_bytes_size);
}

void PackResourceData(const Array<u8>& data, ResourceFile& file) {
  file.data_size = data.GetSize();
  PackBytes(data, file.compression_mode, &file.data, &file.packed_data_size);
}

void UnpackBytes(CompressionMode compression_mode, const u8* packed_bytes,
                 usize packed_bytes_size, usize decompressed_size,
                 Array<u8>& data) {
  switch (compression_mode) {
    case CompressionMode::Lz4: {
      DecompressLz4(packed_bytes, packed_bytes_size, decompressed_size, data);
      break;
    }
    case CompressionMode::None: {
      data.Resize(decompressed_size);
      memory::CopyMemory(data.GetData(), packed_bytes, decompressed_size);
      break;
    }
    default: {
      throw std::runtime_error(
          "Unknown compression mode: " +
          static_cast<std::underlying_type_t<resource::CompressionMode>>(
              compression_mode));
    }
  }
}

void UnpackBytes(CompressionMode compression_mode,
                 const Array<u8>& packed_bytes, usize decompressed_size,
                 Array<u8>& data) {
  UnpackBytes(compression_mode, packed_bytes.GetData(), packed_bytes.GetSize(),
              decompressed_size, data);
}

void UnpackResourceData(const ResourceFile& file, Array<u8>& data) {
  UnpackBytes(file.compression_mode, file.data, file.data_size, data);
}

bool SaveResourceFile(CTStringView path, const ResourceFile& file) {
  std::ofstream out_file;

  if (!OpenFileToWriteTo(path, out_file, false, true)) {
    COMET_LOG_RESOURCE_ERROR("Unable to write resource file: ", path);
    return false;
  }

  out_file.write(reinterpret_cast<const schar*>(&file.resource_type_id),
                 static_cast<std::streamsize>(sizeof(file.resource_type_id)));

  out_file.write(reinterpret_cast<const schar*>(&file.compression_mode),
                 static_cast<std::streamsize>(sizeof(file.compression_mode)));

  out_file.write(reinterpret_cast<const schar*>(&file.packed_descr_size),
                 static_cast<std::streamsize>(sizeof(file.packed_descr_size)));

  out_file.write(reinterpret_cast<const schar*>(&file.packed_data_size),
                 static_cast<std::streamsize>(sizeof(file.packed_data_size)));

  out_file.write(reinterpret_cast<const schar*>(&file.descr_size),
                 static_cast<std::streamsize>(sizeof(file.descr_size)));

  out_file.write(reinterpret_cast<const schar*>(&file.data_size),
                 static_cast<std::streamsize>(sizeof(file.data_size)));

  out_file.write(reinterpret_cast<const schar*>(file.descr.GetData()),
                 file.packed_descr_size);

  out_file.write(reinterpret_cast<const schar*>(file.data.GetData()),
                 file.packed_data_size);

  CloseFile(out_file);
  return true;
}

bool LoadResourceFile(CTStringView path, ResourceFile& file) {
  struct JobParams {
    bool is_loaded{false};
    const tchar* path{nullptr};
    ResourceFile* file{nullptr};
  };

  JobParams params{};
  params.path = path.GetCTStr();
  params.file = &file;

  auto& scheduler{job::Scheduler::Get()};
  auto* counter{scheduler.GenerateCounter()};

  scheduler.KickAndWait(job::GenerateIOJobDescr(
      [](job::IOJobParamsHandle params_handle) {
        auto* params{reinterpret_cast<JobParams*>(params_handle)};
        auto* path{params->path};
        auto* file{params->file};

        std::ifstream in_file;

        if (!OpenFileToReadFrom(path, in_file, false, true)) {
          COMET_LOG_RESOURCE_ERROR("Unable to open resource file: ", path);
          params->is_loaded = false;
          return;
        }

        in_file.seekg(0);
        in_file.read(reinterpret_cast<schar*>(&file->resource_type_id),
                     sizeof(ResourceId));

        in_file.read(reinterpret_cast<schar*>(&file->compression_mode),
                     sizeof(file->compression_mode));

        in_file.read(reinterpret_cast<schar*>(&file->packed_descr_size),
                     sizeof(file->packed_descr_size));

        in_file.read(reinterpret_cast<schar*>(&file->packed_data_size),
                     sizeof(file->packed_data_size));

        in_file.read(reinterpret_cast<schar*>(&file->descr_size),
                     sizeof(file->descr_size));

        in_file.read(reinterpret_cast<schar*>(&file->data_size),
                     sizeof(file->data_size));

        auto& scheduler{job::Scheduler::Get()};
        auto* alloc_counter{scheduler.GenerateCounter()};

        struct AllocJobParams {
          ResourceFile* file{nullptr};
        };

        AllocJobParams alloc_params{};
        alloc_params.file = file;

#ifdef COMET_FIBER_DEBUG_LABEL
        schar debug_label[fiber::Fiber::kDebugLabelMaxLen_ + 1];
        auto prefix_len{GetLength("rfile_alloc_")};
        Copy(debug_label, "rfile_alloc_", prefix_len);
        auto name{GetName(path)};

        Copy(debug_label + prefix_len, name.GetCTStr(),
             math::Min(fiber::Fiber::kDebugLabelMaxLen_ - prefix_len,
                       name.GetLength()));
#else
        const schar* debug_label{nullptr};
#endif  // COMET_FIBER_DEBUG_LABEL

        scheduler.KickAndWait(job::GenerateJobDescr(
            job::JobPriority::High,
            [](job::JobParamsHandle params_handle) {
              auto* alloc_params{
                  reinterpret_cast<AllocJobParams*>(params_handle)};
              auto* file{alloc_params->file};
              file->descr.Resize(file->packed_descr_size);
              file->data.Resize(file->packed_data_size);
            },
            &alloc_params, job::JobStackSize::Normal, alloc_counter,
            debug_label));

        scheduler.DestroyCounter(alloc_counter);

        in_file.read(reinterpret_cast<schar*>(file->descr.GetData()),
                     file->packed_descr_size);

        in_file.read(reinterpret_cast<schar*>(file->data.GetData()),
                     file->packed_data_size);

        params->is_loaded = true;
      },
      &params, counter));

  scheduler.DestroyCounter(counter);
  return params.is_loaded;
}

ResourceHandler::ResourceHandler(usize resource_size,
                                 memory::Allocator* loading_resources_allocator,
                                 memory::Allocator* loading_resource_allocator)

    : resource_allocator_{resource_size, 1024,
                          memory::kEngineMemoryTagResource},
      loading_resources_allocator_{loading_resources_allocator},
      loading_resource_allocator_{loading_resource_allocator} {}

void ResourceHandler::Initialize() {
  resource_allocator_.Initialize();
  cache_allocator_.Initialize();
  loading_resources_ = LoadingResources{loading_resources_allocator_};
  cache_ = ResourceCache{&cache_allocator_, 128};
}

void ResourceHandler::Shutdown() {
  loading_resources_ = {};
  cache_ = {};
  cache_allocator_.Destroy();
  resource_allocator_.Destroy();
}

const Resource* ResourceHandler::Load(memory::Allocator& allocator,
                                      CTStringView root_resource_path,
                                      CTStringView resource_path) {
  return Load(allocator, root_resource_path,
              GenerateResourceIdFromPath(resource_path));
}

const Resource* ResourceHandler::Load(memory::Allocator& allocator,
                                      CTStringView root_resource_path,
                                      ResourceId resource_id) {
#ifdef COMET_PROFILING
  schar label[profiler::kMaxProfileLabelLen + 1]{'\0'};
  constexpr schar kLabelPrefix[]{"ResourceHandler::Load ("};
  constexpr auto kLabelPrefixLen{GetLength(kLabelPrefix)};
  constexpr schar kLabelSuffix[]{")"};
  constexpr auto kLabelSuffixLen{GetLength(kLabelSuffix)};

  Copy(label, kLabelPrefix, kLabelPrefixLen);
  usize out_len;
  ConvertToStr(resource_id, label + kLabelPrefixLen,
               profiler::kMaxProfileLabelLen - kLabelPrefixLen, &out_len);
  Copy(label + kLabelPrefixLen + out_len, kLabelSuffix, kLabelSuffixLen);
  COMET_PROFILE(label);
#endif  // COMET_PROFILING

  if (resource_id == kDefaultResourceId) {
    return GetDefaultResource();
  }

  {
    fiber::FiberLockGuard lock{cache_mutex_};
    auto* resource{GetInternal(resource_id)};

    if (resource != nullptr) {
      ++resource->ref_count;
      return resource;
    }
  }

  internal::LoadingResourceState* state{nullptr};
  bool is_already_loading{false};

  {
    fiber::FiberLockGuard lock{loading_mutex_};
    auto** state_box{loading_resources_.TryGet(resource_id)};

    if (state_box == nullptr) {
      auto* resource_state_ptr{
          loading_resource_allocator_
              ->AllocateOneAndPopulate<internal::LoadingResourceState>()};

      auto& pair{loading_resources_.Emplace(resource_id, resource_state_ptr)};
      state = pair.value;
      state->is_loading = true;
      ++state->ref_count;
    } else {
      state = *state_box;
      ++state->ref_count;
      is_already_loading = true;
    }
  }

  if (is_already_loading) {
    for (;;) {
      {
        fiber::FiberLockGuard lock{loading_mutex_};

        if (!state->is_loading) {
          --state->ref_count;
          auto* resource{state->resource};

          if (state->ref_count == 0) {
            loading_resource_allocator_->Deallocate(
                loading_resources_.Get(resource_id));
            loading_resources_.Remove(resource_id);
          }

          return resource;
        }
      }

      fiber::Yield();
    }
  }

  tchar resource_id_path[10];
  usize resource_id_path_len;
  ConvertToStr(resource_id, resource_id_path, 10, &resource_id_path_len);

  TString resource_abs_path{};
  // Adding 1 for the hypothetical "/" between those two (if needed).
  resource_abs_path.Reserve(root_resource_path.GetLength() + 1 +
                            resource_id_path_len);
  resource_abs_path = root_resource_path;
  resource_abs_path /= resource_id_path;  // No allocation.

  ResourceFile file{};
  file.descr = Array<u8>{&allocator};
  file.data = Array<u8>{&allocator};
  const auto is_load{LoadResourceFile(resource_abs_path, file)};
  Resource* resource;

  if (is_load) {
    resource = Unpack(allocator, file);
    resource->ref_count = 1;

    {
      fiber::FiberLockGuard lock{cache_mutex_};
      cache_.Set(resource->id, resource);
    }
  } else {
    COMET_LOG_RESOURCE_ERROR("Unable to get resource with ID: ",
                             COMET_STRING_ID_LABEL(resource_id), ".");
    resource = GetDefaultResource();
  }

  {
    fiber::FiberLockGuard lock{loading_mutex_};
    auto& state_ref{loading_resources_.Get(resource_id)};

    state_ref->resource = resource;
    state_ref->is_loading = false;
    --state_ref->ref_count;

    if (state_ref->ref_count == 0) {
      loading_resources_.Remove(resource_id);
    }
  }

  return resource;
}

void ResourceHandler::Unload(CTStringView resource_path) {
  Unload(GenerateResourceIdFromPath(resource_path));
}

void ResourceHandler::Unload(ResourceId resource_id) {
  COMET_ASSERT(resource_id != kDefaultResourceId,
               "Cannot unload default resource!");
  fiber::FiberLockGuard lock{cache_mutex_};
  auto* resource{cache_.Get(resource_id)};
  COMET_ASSERT(resource != nullptr, "Tried to unload resource with ID ",
               COMET_STRING_ID_LABEL(resource_id), ", but the former is null!");
  COMET_ASSERT(resource->ref_count > 0, "Resource with ID ",
               COMET_STRING_ID_LABEL(resource_id),
               ", was asked to be unloaded, but reference count is already 0! "
               "What happened??");
  --resource->ref_count;

  if (resource->ref_count < 1) {
    Destroy(resource_id);
  }
}

void ResourceHandler::Destroy(ResourceId resource_id) {
  fiber::FiberLockGuard lock{cache_mutex_};
  auto* resource{cache_.Get(resource_id)};
  resource->~Resource();
  resource_allocator_.Deallocate(resource);
  cache_.Remove(resource_id);
}

Resource* ResourceHandler::GetDefaultResource() { return nullptr; }

ResourceFile ResourceHandler::GetResourceFile(
    memory::Allocator& allocator, const Resource& resource,
    CompressionMode compression_mode) const {
  return Pack(allocator, resource, compression_mode);
}

Resource* ResourceHandler::GetInternal(ResourceId resource_id) {
  return cache_.Get(resource_id);
}
}  // namespace resource
}  // namespace comet
