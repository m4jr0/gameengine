// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_RUNTIME_SHARED_INSTANCE_REGISTRY_H_
#define COMET_RUNTIME_SHARED_INSTANCE_REGISTRY_H_

// External. ///////////////////////////////////////////////////////////////////
#include <functional>
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/container/array.h"
#include "comet/core/container/map.h"
#include "comet/core/essentials.h"
#include "comet/core/fiber/fiber_primitive.h"
#include "comet/core/handle/handle.h"
#include "comet/core/memory/allocator/allocator.h"

namespace comet {
template <typename SourceId, typename HandleTag, typename T>
class SharedInstanceRegistry {
 public:
  using ItemHandle = Handle<HandleTag>;

  struct Slot {
    SourceId source_id{};
    usize ref_count{0};
    T* item{nullptr};
  };

  SharedInstanceRegistry() = default;
  SharedInstanceRegistry(const SharedInstanceRegistry&) = delete;
  SharedInstanceRegistry(SharedInstanceRegistry&&) noexcept = default;
  SharedInstanceRegistry& operator=(const SharedInstanceRegistry&) = delete;
  SharedInstanceRegistry& operator=(SharedInstanceRegistry&&) noexcept =
      default;

  ~SharedInstanceRegistry() {
    COMET_ASSERT(!is_initialized_,
                 "SharedInstanceRegistry::~SharedInstanceRegistry",
                 "shared instance registry is still initialized");
  }

  explicit SharedInstanceRegistry(memory::Allocator* allocator,
                                  usize initial_capacity = 0)
      : slots_{allocator} {
    source_ids_ =
        Map<SourceId, ItemHandle>::WithCapacity(allocator, initial_capacity);
  }

  void Initialize() {
    COMET_ASSERT(!is_initialized_, "SharedInstanceRegistry::Initialize",
                 "shared instance registry is already initialized");
    is_initialized_ = true;
  }

  void Destroy() {
    fiber::FiberLockGuard lock{mtx_};
    COMET_ASSERT(is_initialized_, "SharedInstanceRegistry::Destroy",
                 "shared instance registry is not initialized");
    is_initialized_ = false;
    slots_.Release();
    source_ids_.Release();
    handle_pool_.Destroy();
  }

  ItemHandle TryAcquire(SourceId source_id) {
    fiber::FiberLockGuard lock{mtx_};
    auto* handle_ptr{source_ids_.TryGet(source_id)};

    if (handle_ptr == nullptr) {
      return ItemHandle::Invalid();
    }

    const auto handle{*handle_ptr};

    if (!handle_pool_.IsAlive(handle)) {
      return ItemHandle::Invalid();
    }

    auto& slot{slots_[handle.GetIndex()]};
    COMET_ASSERT(slot.item != nullptr, "SharedInstanceRegistry::TryAcquire",
                 "slot item is null", "handle", handle.GetValue(), "index",
                 handle.GetIndex());

    ++slot.ref_count;
    return handle;
  }

  ItemHandle Create(SourceId source_id, T* item) {
    COMET_ASSERT(item != nullptr, "SharedInstanceRegistry::Create",
                 "item is null");

    fiber::FiberLockGuard lock{mtx_};

    COMET_ASSERT(source_ids_.TryGet(source_id) == nullptr ||
                     !handle_pool_.IsAlive(*source_ids_.TryGet(source_id)),
                 "SharedInstanceRegistry::Create",
                 "item with source id already exists");

    const auto handle{handle_pool_.Generate()};
    const auto index{handle.GetIndex()};

    if (index >= slots_.GetSize()) {
      slots_.Resize(index + 1);
    }

    auto& slot{slots_[index]};
    slot.source_id = source_id;
    slot.ref_count = 1;
    slot.item = item;

    source_ids_.Set(source_id, handle);
    return handle;
  }

  bool Release(ItemHandle handle) {
    fiber::FiberLockGuard lock{mtx_};

    if (!handle_pool_.IsAlive(handle)) {
      return false;
    }

    auto& slot{slots_[handle.GetIndex()]};
    COMET_ASSERT(slot.ref_count > 0, "SharedInstanceRegistry::Release",
                 "item reference count is already zero", "handle",
                 handle.GetValue(), "index", handle.GetIndex());

    --slot.ref_count;
    return slot.ref_count == 0;
  }

  T* TryGet(ItemHandle handle) {
    fiber::FiberLockGuard lock{mtx_};

    if (!handle_pool_.IsAlive(handle)) {
      return nullptr;
    }

    const auto index{handle.GetIndex()};
    COMET_ASSERT(index < slots_.GetSize(), "SharedInstanceRegistry::TryGet",
                 "item handle index is out of bounds", "handle",
                 handle.GetValue(), "index", index, "slot_count",
                 slots_.GetSize());

    return slots_[index].item;
  }

  const T* TryGet(ItemHandle handle) const {
    fiber::FiberLockGuard lock{mtx_};

    if (!handle_pool_.IsAlive(handle)) {
      return nullptr;
    }

    const auto index{handle.GetIndex()};
    COMET_ASSERT(index < slots_.GetSize(), "SharedInstanceRegistry::TryGet",
                 "item handle index is out of bounds", "handle",
                 handle.GetValue(), "index", index, "slot_count",
                 slots_.GetSize());

    return slots_[index].item;
  }

  T* Get(ItemHandle handle) {
    auto* item{TryGet(handle)};
    COMET_ASSERT(item != nullptr, "SharedInstanceRegistry::Get",
                 "item handle is invalid", "handle", handle.GetValue());
    return item;
  }

  const T* Get(ItemHandle handle) const {
    const auto* item{TryGet(handle)};
    COMET_ASSERT(item != nullptr, "SharedInstanceRegistry::Get",
                 "item handle is invalid", "handle", handle.GetValue());
    return item;
  }

  SourceId GetSourceId(ItemHandle handle) const {
    fiber::FiberLockGuard lock{mtx_};

    COMET_ASSERT(handle_pool_.IsAlive(handle),
                 "SharedInstanceRegistry::GetSourceId",
                 "item handle is invalid", "handle", handle.GetValue());

    const auto index{handle.GetIndex()};
    COMET_ASSERT(
        index < slots_.GetSize(), "SharedInstanceRegistry::GetSourceId",
        "item handle index is out of bounds", "handle", handle.GetValue(),
        "index", index, "slot_count", slots_.GetSize());

    return slots_[index].source_id;
  }

  usize GetRefCount(ItemHandle handle) const {
    fiber::FiberLockGuard lock{mtx_};

    if (!handle_pool_.IsAlive(handle)) {
      return 0;
    }

    const auto index{handle.GetIndex()};
    COMET_ASSERT(
        index < slots_.GetSize(), "SharedInstanceRegistry::GetRefCount",
        "item handle index is out of bounds", "handle", handle.GetValue(),
        "index", index, "slot_count", slots_.GetSize());

    return slots_[index].ref_count;
  }

  ItemHandle TryGetHandle(SourceId source_id) const {
    fiber::FiberLockGuard lock{mtx_};

    auto* handle_ptr{source_ids_.TryGet(source_id)};

    if (handle_ptr == nullptr) {
      return ItemHandle::Invalid();
    }

    const auto handle{*handle_ptr};

    if (!handle_pool_.IsAlive(handle)) {
      return ItemHandle::Invalid();
    }

    return handle;
  }

  bool IsAlive(ItemHandle handle) const {
    fiber::FiberLockGuard lock{mtx_};
    return handle_pool_.IsAlive(handle);
  }

  void Remove(ItemHandle handle) {
    fiber::FiberLockGuard lock{mtx_};
    COMET_ASSERT(handle_pool_.IsAlive(handle), "SharedInstanceRegistry::Remove",
                 "item handle is invalid", "handle", handle.GetValue());

    const auto index{handle.GetIndex()};
    auto& slot{slots_[index]};

    COMET_ASSERT(slot.ref_count == 0, "SharedInstanceRegistry::Remove",
                 "cannot remove a live shared instance", "handle",
                 handle.GetValue(), "ref_count", slot.ref_count);

    source_ids_.Remove(slot.source_id);

    slot.source_id = SourceId{};
    slot.ref_count = 0;
    slot.item = nullptr;

    handle_pool_.Destroy(handle);
  }

  T* Drain(ItemHandle handle) {
    fiber::FiberLockGuard lock{mtx_};

    COMET_ASSERT(is_initialized_, "SharedInstanceRegistry::Drain",
                 "shared instance registry is not initialized");

    if (!handle_pool_.IsAlive(handle)) {
      return nullptr;
    }

    const auto index{handle.GetIndex()};
    COMET_ASSERT(index < slots_.GetSize(), "SharedInstanceRegistry::Drain",
                 "item handle index is out of bounds", "handle",
                 handle.GetValue(), "index", index, "slot_count",
                 slots_.GetSize());

    auto& slot{slots_[index]};
    COMET_ASSERT(slot.item != nullptr, "SharedInstanceRegistry::Drain",
                 "slot item is null", "handle", handle.GetValue(), "index",
                 index);

    auto* item{slot.item};

    source_ids_.Remove(slot.source_id);

    slot.source_id = SourceId{};
    slot.ref_count = 0;
    slot.item = nullptr;

    handle_pool_.Destroy(handle);
    return item;
  }

  template <typename Func>
  void ForEachLive(Func&& fn) {
    fiber::FiberLockGuard lock{mtx_};

    for (const auto& pair : source_ids_) {
      const auto handle{pair.value};

      if (!handle_pool_.IsAlive(handle)) {
        continue;
      }

      const auto index{handle.GetIndex()};
      COMET_ASSERT(
          index < slots_.GetSize(), "SharedInstanceRegistry::ForEachLive",
          "item handle index is out of bounds", "handle", handle.GetValue(),
          "index", index, "slot_count", slots_.GetSize());

      auto& slot{slots_[index]};

      if (slot.item == nullptr) {
        continue;
      }

      std::invoke(fn, handle, slot.item);
    }
  }

  template <typename Func>
  void ForEachLive(Func&& fn) const {
    fiber::FiberLockGuard lock{mtx_};

    for (const auto& pair : source_ids_) {
      const auto handle{pair.value};

      if (!handle_pool_.IsAlive(handle)) {
        continue;
      }

      const auto index{handle.GetIndex()};
      COMET_ASSERT(
          index < slots_.GetSize(), "SharedInstanceRegistry::ForEachLive",
          "item handle index is out of bounds", "handle", handle.GetValue(),
          "index", index, "slot_count", slots_.GetSize());

      const auto& slot{slots_[index]};
      if (slot.item == nullptr) {
        continue;
      }

      std::invoke(fn, handle, slot.item);
    }
  }

  usize GetLiveCount() const {
    fiber::FiberLockGuard lock{mtx_};
    return source_ids_.GetEntryCount();
  }

  bool HasNoLiveInstances() const { return GetLiveCount() == 0; }

  bool IsInitialized() const noexcept { return is_initialized_; }

 private:
  bool is_initialized_{false};
  mutable fiber::FiberMutex mtx_{};
  HandlePool<HandleTag> handle_pool_{};
  Array<Slot> slots_{};
  Map<SourceId, ItemHandle> source_ids_{};
};
}  // namespace comet

#endif  // COMET_RUNTIME_SHARED_INSTANCE_REGISTRY_H_
