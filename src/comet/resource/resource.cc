// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "comet_pch.h"

#include "resource.h"

#include <fstream>

#include "comet/core/c_string.h"
#include "comet/core/compression.h"
#include "comet/core/concurrency/job/job.h"
#include "comet/core/concurrency/job/job_utils.h"
#include "comet/core/concurrency/job/scheduler.h"
#include "comet/core/file_system/file_system.h"
#include "comet/core/logger.h"
#include "comet/core/memory/memory_utils.h"
#include "comet/math/math_common.h"

namespace comet {
namespace resource {
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
      packed_bytes->Resize(bytes_size);
      memory::CopyMemory(packed_bytes->GetData(), bytes, bytes_size);
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

void UnpackResourceData(const ResourceFile& file, Array<u8>& data,
                        usize max_data_size) {
  auto data_size{max_data_size != kInvalidSize
                     ? math::Min(max_data_size, file.data_size)
                     : file.data_size};

  UnpackBytes(file.compression_mode, file.data, data_size, data);
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

  job::CounterGuard guard{};

  job::Scheduler::Get().KickAndWait(job::GenerateIOJobDescr(
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

        {
          job::CounterGuard alloc_guard{};

          job::Scheduler::Get().KickAndWait(job::GenerateJobDescr(
              job::JobPriority::High,
              [](job::JobParamsHandle params_handle) {
                auto* alloc_params{
                    reinterpret_cast<AllocJobParams*>(params_handle)};
                auto* file{alloc_params->file};
                file->descr.Resize(file->packed_descr_size);
                file->data.Resize(file->packed_data_size);
              },
              &alloc_params, job::JobStackSize::Normal,
              alloc_guard.GetCounter(), debug_label));
        }

        in_file.read(reinterpret_cast<schar*>(file->descr.GetData()),
                     file->packed_descr_size);

        in_file.read(reinterpret_cast<schar*>(file->data.GetData()),
                     file->packed_data_size);

        params->is_loaded = true;
      },
      &params, guard.GetCounter()));

  return params.is_loaded;
}
}  // namespace resource
}  // namespace comet
