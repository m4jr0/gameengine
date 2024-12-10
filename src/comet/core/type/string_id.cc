// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "string_id.h"

#include <memory>

#include "comet/core/c_string.h"
#include "comet/core/generator.h"
#include "comet/core/hash.h"
#include "comet/core/memory/memory_utils.h"

#ifdef COMET_LABELIZE_STRING_IDS
#include <atomic>
#include <mutex>
#include <shared_mutex>

#include "comet/core/memory/memory_utils.h"
#include "comet/core/type/map.h"
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
    label_table.Clear();
    string_id_allocator.Destroy();
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

StringIdAllocator::~StringIdAllocator() {
  COMET_ASSERT(
      !is_initialized_,
      "Destructor called for aligned allocator, but it is still initialized!");
}

void StringIdAllocator::Initialize() {
  COMET_ASSERT(
      !is_initialized_,
      "Tried to initialize aligned allocator, but it is already done!");
  offset_ = 0;
  root_ = static_cast<u8*>(
      memory::AllocateAligned(sizeof(schar) * capacity_, alignof(schar),
                              memory::kEngineMemoryTagStringId));
  is_initialized_ = true;
}

void StringIdAllocator::Destroy() {
  COMET_ASSERT(
      is_initialized_,
      "Tried to shutdown aligned allocator, but it is not initialized!");
  memory::Deallocate(root_);
  offset_ = kInvalidOffset_;
  root_ = nullptr;
  is_initialized_ = false;
}

void* StringIdAllocator::AllocateAligned(usize size, memory::Alignment align) {
  COMET_ASSERT(size > 0, "Allocation size provided is 0!");
  auto current_offset{offset_.load(std::memory_order_relaxed)};
  COMET_ASSERT(current_offset >= 0, "Invalid offset: ", current_offset, "!");
  auto aligned_offset{
      memory::AlignAddress(reinterpret_cast<uptr>(root_ + current_offset),
                           align) -
      reinterpret_cast<uptr>(root_)};
  auto new_offset{aligned_offset + size};

  COMET_ASSERT(new_offset < capacity_, "Could not allocate enough memory (",
               size, ")!");

  while (!offset_.compare_exchange_weak(current_offset, new_offset,
                                        std::memory_order_acquire)) {
    COMET_ASSERT(current_offset >= 0, "Invalid offset: ", current_offset, "!");
    aligned_offset = memory::AlignAddress(
        reinterpret_cast<uptr>(root_ + current_offset), align);
    new_offset = aligned_offset + size;
    COMET_ASSERT(new_offset > capacity_, "Could not allocate enough memory (",
                 size, ")!");
  }

  return root_ + aligned_offset;
}

void StringIdAllocator::Deallocate(void*) {
  // A stack allocator does not support individual deallocations, as it is
  // intended for temporary data only. Memory is only released when Clear() is
  // called, which resets the entire stack.
}

void StringIdAllocator::Clear() { offset_.store(0, std::memory_order_release); }

void StringIdAllocator::Reset() {}

bool StringIdAllocator::IsInitialized() const noexcept {
  return is_initialized_;
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

  if (debug_data.label_table.GetEntryCount() == 0) {
    return;
  }

  debug_data.label_table.Clear();

  if (debug_data.string_id_allocator.IsInitialized()) {
    debug_data.string_id_allocator.Destroy();
  }
#endif  // COMET_LABELIZE_STRING_IDS
}

StringId StringIdHandler::Generate(const schar* str, usize length) {
  COMET_ASSERT(str != nullptr,
               "String provided is null! Cannot generate string ID.");
  COMET_ASSERT(length > 0, "Cannot generate ID from empty string.");
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
  COMET_ASSERT(length > 0, "Length provided is 0! Cannot generate String ID !");
  return Generate(GenerateForOneFrame<schar>(str, length), length);
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
    auto* placeholder{GenerateForOneFrame<schar>(12)};
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
  static std::unique_ptr<stringid::StringIdHandler> string_id_handler{nullptr};

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
