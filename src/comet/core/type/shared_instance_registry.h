// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_TYPE_SHARED_INSTANCE_REGISTRY_H_
#define COMET_COMET_CORE_TYPE_SHARED_INSTANCE_REGISTRY_H_

// External. ///////////////////////////////////////////////////////////////////
#include <functional>
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/concurrency/fiber/fiber_primitive.h"
#include "comet/core/essentials.h"
#include "comet/core/memory/allocator/allocator.h"
#include "comet/core/type/array.h"
#include "comet/core/type/handle.h"
#include "comet/core/type/map.h"

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
    COMET_ASSERT(
        !is_initialized_,
        "Destructor called for shared instance registry, but it is still "
        "initialized!");
  }

  explicit SharedInstanceRegistry(memory::Allocator* allocator,
                                  usize initial_capacity = 0)
      : slots_{allocator}, source_ids_{allocator, initial_capacity} {}

  void Initialize() {
    COMET_ASSERT(!is_initialized_,
                 "Tried to initialize shared instance registry, but it is "
                 "already done!");
    is_initialized_ = true;
  }

  void Destroy() {
    fiber::FiberLockGuard lock{mtx_};
    COMET_ASSERT(is_initialized_,
                 "Tried to destroy shared instance registry, but it is not "
                 "initialized!");
    is_initialized_ = false;
    slots_.Destroy();
    source_ids_.Destroy();
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
    COMET_ASSERT(slot.item != nullptr, "Slot item is null!");
    ++slot.ref_count;
    return handle;
  }

  ItemHandle Create(SourceId source_id, T* item) {
    COMET_ASSERT(item != nullptr, "Item provided is null!");
    fiber::FiberLockGuard lock{mtx_};

    COMET_ASSERT(source_ids_.TryGet(source_id) == nullptr ||
                     !handle_pool_.IsAlive(*source_ids_.TryGet(source_id)),
                 "Item with source ID already exists!");

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
    COMET_ASSERT(slot.ref_count > 0, "Item reference count is already 0!");

    --slot.ref_count;
    return slot.ref_count == 0;
  }

  T* TryGet(ItemHandle handle) {
    fiber::FiberLockGuard lock{mtx_};

    if (!handle_pool_.IsAlive(handle)) {
      return nullptr;
    }

    const auto index{handle.GetIndex()};
    COMET_ASSERT(index < slots_.GetSize(),
                 "Item handle index is out of bounds!");
    return slots_[index].item;
  }

  const T* TryGet(ItemHandle handle) const {
    fiber::FiberLockGuard lock{mtx_};

    if (!handle_pool_.IsAlive(handle)) {
      return nullptr;
    }

    const auto index{handle.GetIndex()};
    COMET_ASSERT(index < slots_.GetSize(),
                 "Item handle index is out of bounds!");
    return slots_[index].item;
  }

  T* Get(ItemHandle handle) {
    auto* item{TryGet(handle)};
    COMET_ASSERT(item != nullptr, "Item handle is invalid!");
    return item;
  }

  const T* Get(ItemHandle handle) const {
    const auto* item{TryGet(handle)};
    COMET_ASSERT(item != nullptr, "Item handle is invalid!");
    return item;
  }

  SourceId GetSourceId(ItemHandle handle) const {
    fiber::FiberLockGuard lock{mtx_};

    COMET_ASSERT(handle_pool_.IsAlive(handle), "Item handle is invalid!");
    const auto index{handle.GetIndex()};
    COMET_ASSERT(index < slots_.GetSize(),
                 "Item handle index is out of bounds!");
    return slots_[index].source_id;
  }

  usize GetRefCount(ItemHandle handle) const {
    fiber::FiberLockGuard lock{mtx_};

    if (!handle_pool_.IsAlive(handle)) {
      return 0;
    }

    const auto index{handle.GetIndex()};
    COMET_ASSERT(index < slots_.GetSize(),
                 "Item handle index is out of bounds!");
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
    COMET_ASSERT(handle_pool_.IsAlive(handle), "Item handle is invalid!");

    const auto index{handle.GetIndex()};
    auto& slot{slots_[index]};

    COMET_ASSERT(
        slot.ref_count == 0,
        "Tried to remove a live shared instance with reference count of ",
        slot.ref_count, "!");

    source_ids_.Remove(slot.source_id);

    slot.source_id = SourceId{};
    slot.ref_count = 0;
    slot.item = nullptr;

    handle_pool_.Destroy(handle);
  }

  T* Drain(ItemHandle handle) {
    fiber::FiberLockGuard lock{mtx_};

    COMET_ASSERT(
        is_initialized_,
        "Tried to drain shared instance registry, but it is not initialized!");

    if (!handle_pool_.IsAlive(handle)) {
      return nullptr;
    }

    const auto index{handle.GetIndex()};
    COMET_ASSERT(index < slots_.GetSize(),
                 "Item handle index is out of bounds!");

    auto& slot{slots_[index]};
    COMET_ASSERT(slot.item != nullptr, "Slot item is null!");

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
      COMET_ASSERT(index < slots_.GetSize(),
                   "Item handle index is out of bounds!");

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
      COMET_ASSERT(index < slots_.GetSize(),
                   "Item handle index is out of bounds!");

      const auto& slot{slots_[index]};
      if (slot.item == nullptr) {
        continue;
      }

      std::invoke(fn, handle, slot.item);
    }
  }

  bool IsInitialized() const noexcept { return is_initialized_; }

 private:
  bool is_initialized_{false};
  mutable fiber::FiberMutex mtx_{};
  HandlePool<HandleTag> handle_pool_{};
  Array<Slot> slots_{};
  Map<SourceId, ItemHandle> source_ids_{};
};
}  // namespace comet

#endif  // COMET_COMET_CORE_TYPE_SHARED_INSTANCE_REGISTRY_H_
