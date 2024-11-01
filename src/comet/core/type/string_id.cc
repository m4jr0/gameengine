// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "string_id.h"

#include <memory>

#include "comet/core/c_string.h"
#include "comet/core/generator.h"
#include "comet/core/hash.h"
#include "comet/core/memory/memory_general_alloc.h"

#ifdef COMET_LABELIZE_STRING_IDS
#include <unordered_map>
#endif  // COMET_LABELIZE_STRING_IDS

namespace comet {
namespace stringid {
#ifdef COMET_LABELIZE_STRING_IDS
namespace internal {
struct DebugData {
  // TODO(m4jr0): Find lock-free solution.
  std::unordered_map<StringId, schar*> string_id_table{};
  StringIdAllocator string_id_allocator_{2097152};  // 2 MiB.
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

void* StringIdAllocator::Allocate(usize size) {
  auto current_offset{offset_.load(std::memory_order_relaxed)};
  COMET_ASSERT(current_offset >= 0, "Invalid offset: ", current_offset, "!");
  auto aligned_offset{memory::AlignAddress(current_offset, alignof(schar))};
  auto new_offset{aligned_offset + size};

  COMET_ASSERT(new_offset <= capacity_, "Could not allocate enough memory (",
               size, ")!");

  while (!offset_.compare_exchange_weak(current_offset, new_offset,
                                        std::memory_order_acquire)) {
    COMET_ASSERT(current_offset >= 0, "Invalid offset: ", current_offset, "!");
    aligned_offset = memory::AlignAddress(current_offset, alignof(schar));
    new_offset = aligned_offset + size;
    COMET_ASSERT(new_offset <= capacity_, "Could not allocate enough memory (",
                 size, ")!");
  }

  return root_ + aligned_offset;
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
  if (internal::GetDebugData().string_id_table.size() == 0) {
    return;
  }

  internal::GetDebugData().string_id_table.clear();

  if (internal::GetDebugData().string_id_allocator_.IsInitialized()) {
    internal::GetDebugData().string_id_allocator_.Destroy();
  }
#endif  // COMET_LABELIZE_STRING_IDS
}

StringId StringIdHandler::Generate(const schar* str, usize length) {
  COMET_ASSERT(str != nullptr,
               "String provided is null! Cannot generate string ID.");
  COMET_ASSERT(length > 0, "Cannot generate ID from empty string.");
  const auto string_id{HashCrC32(str, length)};

#ifdef COMET_LABELIZE_STRING_IDS
  const auto it{internal::GetDebugData().string_id_table.find(string_id)};

  if (it == internal::GetDebugData().string_id_table.end()) {
    if (!internal::GetDebugData().string_id_allocator_.IsInitialized()) {
      internal::GetDebugData().string_id_allocator_.Initialize();
    }

    auto* saved_str{reinterpret_cast<schar*>(
        internal::GetDebugData().string_id_allocator_.Allocate(length + 1))};
    Copy(saved_str, str, length);
    saved_str[length] = '\0';
    internal::GetDebugData().string_id_table[string_id] = saved_str;
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
  const auto it{internal::GetDebugData().string_id_table.find(string_id)};

  if (it == internal::GetDebugData().string_id_table.end()) {
#endif  // COMET_LABELIZE_STRING_IDS
    auto* label{GenerateForOneFrame<schar>(12)};
    label[0] = '?';
    ConvertToStr(string_id, label + 1, 12);
    label[11] = '?';
    label[12] = '\0';
    return label;
#ifdef COMET_LABELIZE_STRING_IDS
  }

  return internal::GetDebugData().string_id_table.at(string_id);
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
