// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "string_id.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/c_string.h"
#include "comet/core/frame/frame_string.h"
#include "comet/core/hash.h"
#include "comet/core/memory/memory.h"

#ifdef COMET_LABELIZE_STRING_IDS
#include <atomic>
#include <mutex>
#include <shared_mutex>

#include "comet/core/memory/memory_utils.h"
#include "comet/core/type/map.h"

#if defined(COMET_DEBUG_STRING_ID_ALLOCATOR) && \
    defined(COMET_VERBOSE_ALLOCATOR_LOGS)
#include "comet/core/logger/logging.h"
#endif  // defined(COMET_DEBUG_STRING_ID_ALLOCATOR) &&
        //  defined(COMET_VERBOSE_ALLOCATOR_LOGS)
#endif  // COMET_LABELIZE_STRING_IDS

namespace comet {
namespace stringid {
#ifdef COMET_LABELIZE_STRING_IDS
namespace internal {
struct DebugData {
  StringIdAllocator string_id_allocator{2097152};  // 2 MiB.
  // TODO(m4jr0): Consider lock-free solution.
  Map<StringId, schar*> label_table{};
  std::shared_mutex label_mutex{};

  void Initialize() {
    string_id_allocator.Initialize();
    label_table = Map<StringId, schar*>{&string_id_allocator};
  }

  void Destroy() {
    label_table.Release();

    if (string_id_allocator.IsInitialized()) {
      string_id_allocator.Destroy();
    }
  }

  bool IsInitialized() const noexcept {
    return string_id_allocator.IsInitialized();
  }

  void InitializeIfNeeded() {
    if (IsInitialized()) {
      return;
    }

    Initialize();
  }
};

StringIdAllocator::StringIdAllocator(usize capacity)
    : capacity_{capacity}, offset_{kInvalidOffset_}, root_{nullptr} {}

void* StringIdAllocator::AllocateAligned(usize size, memory::Alignment align) {
  COMET_ASSERT(IsInitialized(), "StringIdAllocator::AllocateAligned",
               "allocator is not initialized");
  COMET_ASSERT(root_ != nullptr, "StringIdAllocator::AllocateAligned",
               "allocator root is null");
  COMET_ASSERT(capacity_ > 0, "StringIdAllocator::AllocateAligned",
               "capacity is invalid", "capacity", capacity_);
  COMET_ASSERT(align > 0, "StringIdAllocator::AllocateAligned",
               "alignment is invalid", "align", align);
  COMET_ASSERT(size > 0, "StringIdAllocator::AllocateAligned",
               "allocation size is zero");

  auto current_offset{offset_.load(std::memory_order_relaxed)};
  COMET_ASSERT(current_offset >= 0, "StringIdAllocator::AllocateAligned",
               "offset is invalid", "offset", current_offset);

  auto aligned_offset{
      memory::AlignAddress(reinterpret_cast<uptr>(root_ + current_offset),
                           align) -
      reinterpret_cast<uptr>(root_)};
  auto new_offset{aligned_offset + size};

  COMET_ASSERT(new_offset <= capacity_, "StringIdAllocator::AllocateAligned",
               "allocation exceeds capacity", "size", size, "new_offset",
               new_offset, "capacity", capacity_);

  while (!offset_.compare_exchange_weak(current_offset, new_offset,
                                        std::memory_order_acq_rel,
                                        std::memory_order_relaxed)) {
    COMET_ASSERT(current_offset >= 0, "StringIdAllocator::AllocateAligned",
                 "offset is invalid", "offset", current_offset);

    aligned_offset =
        memory::AlignAddress(reinterpret_cast<uptr>(root_ + current_offset),
                             align) -
        reinterpret_cast<uptr>(root_);
    new_offset = aligned_offset + size;

    COMET_ASSERT(new_offset <= capacity_, "StringIdAllocator::AllocateAligned",
                 "allocation exceeds capacity", "size", size, "new_offset",
                 new_offset, "capacity", capacity_);
  }

#ifdef COMET_DEBUG_STRING_ID_ALLOCATOR
  allocation_count_.fetch_add(1, std::memory_order_relaxed);
  auto peak{peak_used_size_.load(std::memory_order_relaxed)};

  while (new_offset > peak &&
         !peak_used_size_.compare_exchange_weak(peak, new_offset,
                                                std::memory_order_relaxed)) {
  }
#endif  // COMET_DEBUG_STRING_ID_ALLOCATOR

  return root_ + aligned_offset;
}

void StringIdAllocator::Deallocate(void*) {
  // A stack allocator does not support individual deallocations, as it is
  // intended for temporary data only. Memory is only released when Clear() is
  // called, which resets the entire stack.
}

void StringIdAllocator::Clear() {
  offset_.store(0, std::memory_order_release);

#ifdef COMET_DEBUG_STRING_ID_ALLOCATOR
  clear_count_.fetch_add(1, std::memory_order_relaxed);
#endif  // COMET_DEBUG_STRING_ID_ALLOCATOR
}

void StringIdAllocator::Reset() {
#ifdef COMET_DEBUG_STRING_ID_ALLOCATOR
  allocation_count_.store(0, std::memory_order_relaxed);
  peak_used_size_.store(0, std::memory_order_relaxed);
  clear_count_.store(0, std::memory_order_relaxed);
#endif  // COMET_DEBUG_STRING_ID_ALLOCATOR
}

void StringIdAllocator::OnInitialize() {
  COMET_ASSERT(capacity_ > 0, "StringIdAllocator::OnInitialize",
               "capacity is invalid", "capacity", capacity_);
  offset_ = 0;
  root_ = static_cast<u8*>(
      memory::AllocateAligned(sizeof(schar) * capacity_, alignof(schar),
                              memory::kEngineMemoryTagStringId));

  COMET_ASSERT(root_ != nullptr, "StringIdAllocator::OnInitialize",
               "failed to allocate string id allocator memory", "capacity",
               capacity_);

#ifdef COMET_DEBUG_STRING_ID_ALLOCATOR
  Reset();
#endif  // COMET_DEBUG_STRING_ID_ALLOCATOR
}

void StringIdAllocator::OnDestroy() {
#if defined(COMET_DEBUG_STRING_ID_ALLOCATOR) && \
    defined(COMET_VERBOSE_ALLOCATOR_LOGS)
  COMET_LOG_DEBUG(
      LoggerType::Core, "StringIdAllocator::OnDestroy",
      "destroy string id allocator", "capacity", capacity_, "allocation_count",
      allocation_count_.load(std::memory_order_relaxed), "clear_count",
      clear_count_.load(std::memory_order_relaxed), "peak_used_size",
      peak_used_size_.load(std::memory_order_relaxed));
#endif  // defined(COMET_DEBUG_STRING_ID_ALLOCATOR) &&
        //  defined(COMET_VERBOSE_ALLOCATOR_LOGS)

  if (root_ != nullptr) {
    memory::Deallocate(root_);
  }

  offset_ = kInvalidOffset_;
  root_ = nullptr;

#ifdef COMET_DEBUG_STRING_ID_ALLOCATOR
  Reset();
#endif  // COMET_DEBUG_STRING_ID_ALLOCATOR
}

static DebugData& GetDebugData() {
  static DebugData debug_data{};
  return debug_data;
}
}  // namespace internal
#endif  // COMET_LABELIZE_STRING_IDS

StringIdHandler::~StringIdHandler() {
#ifdef COMET_LABELIZE_STRING_IDS
  auto& debug_data{internal::GetDebugData()};
  std::unique_lock<std::shared_mutex> lock{debug_data.label_mutex};
  debug_data.Destroy();
#endif  // COMET_LABELIZE_STRING_IDS
}

StringId StringIdHandler::Generate(const schar* str, usize length) {
  COMET_ASSERT(str != nullptr, "StringIdHandler::Generate", "string is null");
  COMET_ASSERT(length > 0, "StringIdHandler::Generate",
               "string length is zero");
  const auto string_id{HashCrC32(str, length)};

#ifdef COMET_LABELIZE_STRING_IDS
  auto& debug_data{internal::GetDebugData()};
  std::unique_lock<std::shared_mutex> lock{debug_data.label_mutex};
  debug_data.InitializeIfNeeded();

  if (!debug_data.label_table.IsContained(string_id)) {
    auto* saved_str{
        debug_data.string_id_allocator.AllocateMany<schar>(length + 1)};
    Copy(saved_str, str, length);
    saved_str[length] = '\0';
    debug_data.label_table.Emplace(string_id, saved_str);
  }
#endif  // COMET_LABELIZE_STRING_IDS

  return string_id;
}

StringId StringIdHandler::Generate(const wchar* str, usize length) {
  COMET_ASSERT(str != nullptr, "StringIdHandler::Generate", "string is null");
  COMET_ASSERT(length > 0, "StringIdHandler::Generate",
               "string length is zero");
  return Generate(GenerateFrameString<schar>(str, length), length);
}

StringId StringIdHandler::Generate(const schar* str) {
  return Generate(str, GetLength(str));
}

StringId StringIdHandler::Generate(const wchar* str) {
  return Generate(str, GetLength(str));
}

// Return temporary string for debug purposes. The schar* returned SHOULD NOT be
// stored.
const schar* StringIdHandler::Labelize(StringId string_id) const {
#ifdef COMET_LABELIZE_STRING_IDS
  auto& debug_data{internal::GetDebugData()};
  std::shared_lock<std::shared_mutex> lock{debug_data.label_mutex};
  debug_data.InitializeIfNeeded();
  const auto* label{debug_data.label_table.TryGet(string_id)};

  if (label == nullptr) {
#endif  // COMET_LABELIZE_STRING_IDS
    auto* placeholder{GenerateFrameString<schar>(12)};
    placeholder[0] = '?';
    ConvertToStr(string_id, placeholder + 1, 12);
    placeholder[11] = '?';
    placeholder[12] = '\0';
    return placeholder;
#ifdef COMET_LABELIZE_STRING_IDS
  }

  return *label;
#endif  // COMET_LABELIZE_STRING_IDS
}

StringIdHandler* SetHandler(bool is_destroy) {
  static memory::UniquePtr<stringid::StringIdHandler> string_id_handler{
      nullptr};

  if (is_destroy) {
    string_id_handler = nullptr;
    return string_id_handler.get();
  }

  if (string_id_handler == nullptr) {
    string_id_handler = std::make_unique<stringid::StringIdHandler>();
  }

  return string_id_handler.get();
}
}  // namespace stringid
}  // namespace comet
