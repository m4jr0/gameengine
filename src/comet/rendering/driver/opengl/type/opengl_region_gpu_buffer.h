// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_OPENGL_TYPE_OPENGL_REGION_GPU_BUFFER_H_
#define COMET_COMET_RENDERING_DRIVER_OPENGL_TYPE_OPENGL_REGION_GPU_BUFFER_H_

// External. ///////////////////////////////////////////////////////////////////
#include "glad/glad.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/essentials.h"
#include "comet/core/logger/logging.h"
#include "comet/core/memory/allocator/allocator.h"
#include "comet/core/memory/memory_utils.h"
#include "comet/core/type/region_map.h"
#include "comet/profiler/profiler.h"
#include "comet/rendering/driver/opengl/opengl_debug.h"
#include "comet/rendering/driver/opengl/type/opengl_storage.h"

namespace comet {
namespace rendering {
namespace gl {
struct GpuBufferCopyRegion {
  GLsizeiptr src_offset{0};
  GLsizeiptr dst_offset{0};
  GLsizei size{0};
};

template <typename T>
struct RegionGpuBuffer {
  RegionGpuBuffer() = default;

  RegionGpuBuffer(
      memory::Allocator* allocator, GLenum bind_target,
      usize element_block_count, usize element_count = 0, GLbitfield flags = 0,
      [[maybe_unused]] const schar* debug_label = "region_gpu_buffer")
      : bind_target_{bind_target},
        storage_flags_{flags | GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT |
                       GL_MAP_COHERENT_BIT},
        element_block_count_{element_block_count},
        element_count_{element_count},
        region_map_{allocator, element_block_count_ * sizeof(T),
                    element_count_ * sizeof(T)} {
#ifdef COMET_RENDERING_USE_DEBUG_LABELS
    if (debug_label == nullptr) {
      debug_label = "";
    }

    const auto debug_label_len{
        math::Min(GetLength(debug_label), kMaxDebugLabelLen_)};
    Copy(debug_label_, debug_label, debug_label_len);
#endif  // COMET_RENDERING_USE_DEBUG_LABELS
  }

  RegionGpuBuffer(const RegionGpuBuffer&) = delete;
  RegionGpuBuffer(RegionGpuBuffer&&) = default;
  RegionGpuBuffer& operator=(const RegionGpuBuffer&) = delete;
  RegionGpuBuffer& operator=(RegionGpuBuffer&&) = default;

  ~RegionGpuBuffer() {
    COMET_ASSERT(!is_initialized_, "RegionGpuBuffer::~RegionGpuBuffer",
                 "region gpu buffer still initialized");
  }

  void Initialize() {
    COMET_ASSERT(!is_initialized_, "RegionGpuBuffer::Initialize",
                 "region gpu buffer already initialized");

    Resize(element_count_);
    is_initialized_ = true;
  }

  void Destroy() {
    COMET_ASSERT(is_initialized_, "RegionGpuBuffer::Destroy",
                 "region gpu buffer not initialized");

    region_map_.Destroy();

    if (storage_native_handle_ != kInvalidGlNativeStorageHandle) {
      glBindBuffer(bind_target_, storage_native_handle_);
      glUnmapBuffer(bind_target_);
      glDeleteBuffers(1, &storage_native_handle_);
      storage_native_handle_ = kInvalidGlNativeStorageHandle;
      mapped_memory_ = nullptr;
    }

    is_initialized_ = false;
  }

  GLint Claim(usize claimed_count) {
    COMET_PROFILE("RegionGpuBuffer<T>::Claim");

    const auto claimed_size{claimed_count * sizeof(T)};
    auto block_offset{region_map_.Claim(claimed_size)};

    if (block_offset == kInvalidSize) {
      const auto new_element_count{
          math::Max(element_count_ + claimed_count, element_count_ * 2)};

      COMET_LOG_WARNING(LoggerType::Rendering, "RegionGpuBuffer::Claim",
                        "region gpu buffer resize required", "old_size",
                        element_count_ * sizeof(T), "new_size",
                        new_element_count * sizeof(T));

      Resize(new_element_count);
      block_offset = region_map_.Claim(claimed_size);
    }

    COMET_ASSERT(block_offset != kInvalidSize, "RegionGpuBuffer::Claim",
                 "region gpu buffer is out of memory", "claimed_count",
                 claimed_count, "element_size", sizeof(T));

    return static_cast<GLint>(block_offset / sizeof(T));
  }

  void Release(usize index_offset, usize released_count) {
    COMET_PROFILE("RegionGpuBuffer<T>::Release");
    region_map_.Release(index_offset * sizeof(T), released_count * sizeof(T));
  }

  usize CheckOrMove(GLint old_index_offset, usize old_count, usize new_count) {
    COMET_PROFILE("RegionGpuBuffer<T>::CheckOrMove");

    if (old_count >= new_count) {
      return old_index_offset;
    }

    Release(old_index_offset, old_count);
    return Claim(new_count);
  }

  void Bind() {
    COMET_ASSERT(storage_native_handle_ != kInvalidGlNativeStorageHandle,
                 "RegionGpuBuffer::Bind", "storage handle invalid");
    glBindBuffer(bind_target_, storage_native_handle_);
  }

  void Unbind() { glBindBuffer(bind_target_, kInvalidGlNativeStorageHandle); }

  bool Upload(const void* uploaded_data,
              const Array<GpuBufferCopyRegion>& copy_regions) {
    COMET_PROFILE("RegionGpuBuffer<T>::Upload");
    COMET_ASSERT(mapped_memory_ != nullptr, "RegionGpuBuffer::Upload",
                 "mapped memory is null");

    if (copy_regions.IsEmpty()) {
      return false;
    }

    const auto* ptr{static_cast<const u8*>(uploaded_data)};

    for (const auto& region : copy_regions) {
      memory::CopyMemory(static_cast<u8*>(mapped_memory_) + region.dst_offset,
                         ptr + region.src_offset, region.size);
    }

    return true;
  }

  void Resize(usize new_element_count) {
    COMET_PROFILE("RegionGpuBuffer<T>::Resize");

    new_element_count =
        memory::RoundUpToMultiple(new_element_count, element_block_count_);

    if (new_element_count <= element_count_ && is_initialized_) {
      return;
    }

    const auto old_storage_size{element_count_ * sizeof(T)};
    const auto old_storage_handle{storage_native_handle_};

    element_count_ = new_element_count;

    if (old_storage_handle != kInvalidGlNativeStorageHandle &&
        mapped_memory_ != nullptr) {
      Bind();
      glUnmapBuffer(bind_target_);
      Unbind();
    }

    glGenBuffers(1, &storage_native_handle_);
    glBindBuffer(bind_target_, storage_native_handle_);

#ifdef COMET_RENDERING_USE_DEBUG_LABELS
    if (!IsEmpty(debug_label_)) {
      COMET_GL_SET_UNIFORM_BUFFER_DEBUG_LABEL(storage_native_handle_,
                                              debug_label_);
    }
#endif  // COMET_RENDERING_USE_DEBUG_LABELS

    const auto storage_size{element_count_ * sizeof(T)};
    glBufferStorage(bind_target_, static_cast<GLsizei>(storage_size), nullptr,
                    storage_flags_);

    mapped_memory_ = glMapBufferRange(
        bind_target_, 0, static_cast<GLsizei>(storage_size), storage_flags_);

    COMET_ASSERT(mapped_memory_ != nullptr, "RegionGpuBuffer::Resize",
                 "buffer mapping failed", "element_count", element_count_,
                 "storage_size", storage_size);

    if (old_storage_handle != kInvalidGlNativeStorageHandle) {
      glBindBuffer(GL_COPY_READ_BUFFER, old_storage_handle);
      glBindBuffer(GL_COPY_WRITE_BUFFER, storage_native_handle_);
      glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0,
                          old_storage_size);
      glDeleteBuffers(1, &old_storage_handle);
    }

    glBindBuffer(bind_target_, kInvalidGlNativeStorageHandle);
    region_map_.Resize(storage_size);
  }

  GlNativeStorageHandle GetHandle() const noexcept {
    return storage_native_handle_;
  }

 private:
  bool is_initialized_{false};
  GLenum bind_target_{0};
  GlNativeStorageHandle storage_native_handle_{kInvalidGlNativeStorageHandle};
  GLbitfield storage_flags_{0};
  usize element_block_count_{0};
  usize element_count_{0};
  RegionMap region_map_{};
  void* mapped_memory_{nullptr};

#ifdef COMET_RENDERING_USE_DEBUG_LABELS
  inline static constexpr usize kMaxDebugLabelLen_{31};
  schar debug_label_[kMaxDebugLabelLen_ + 1]{'\0'};
#endif  // COMET_RENDERING_USE_DEBUG_LABELS
};

struct VertexGpuBuffer : public RegionGpuBuffer<geometry::SkinnedVertex> {
 public:
  VertexGpuBuffer() : VertexGpuBuffer(nullptr, kDefaultElementCount_) {}

  VertexGpuBuffer(
      memory::Allocator* allocator,
      usize element_block_count = kDefaultElementCount_,
      usize element_count = 0, GLbitfield flags = 0,
      [[maybe_unused]] const schar* debug_label = "vertex_gpu_buffer")
      : RegionGpuBuffer<geometry::SkinnedVertex>(
            allocator, GL_ARRAY_BUFFER, element_block_count, element_count,
            flags, debug_label) {}

 private:
  inline static constexpr usize kDefaultElementCount_{8192};
};

struct IndexGpuBuffer : public RegionGpuBuffer<geometry::Index> {
 public:
  IndexGpuBuffer() : IndexGpuBuffer(nullptr, kDefaultElementCount_) {}

  IndexGpuBuffer(memory::Allocator* allocator,
                 usize element_block_count = kDefaultElementCount_,
                 usize element_count = 0, GLbitfield flags = 0,
                 [[maybe_unused]] const schar* debug_label = "index_gpu_buffer")
      : RegionGpuBuffer<geometry::Index>(allocator, GL_ELEMENT_ARRAY_BUFFER,
                                         element_block_count, element_count,
                                         flags, debug_label) {}

 private:
  inline static constexpr usize kDefaultElementCount_{16384};
};
}  // namespace gl
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_OPENGL_TYPE_OPENGL_REGION_GPU_BUFFER_H_