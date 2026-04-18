// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RESOURCE_RUNTIME_RESOURCE_SLOTS_H_
#define COMET_COMET_RESOURCE_RUNTIME_RESOURCE_SLOTS_H_

// External. ///////////////////////////////////////////////////////////////////
#include <type_traits>
#include <utility>
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/concurrency/fiber/fiber_primitive.h"
#include "comet/core/essentials.h"
#include "comet/core/memory/allocator/allocator.h"
#include "comet/core/type/array.h"
#include "comet/core/type/handle.h"
#include "comet/core/type/map.h"
#include "comet/resource/handler/resource_handler_utils.h"
#include "comet/resource/resource.h"
#include "comet/resource/resource_id.h"
#include "comet/resource/runtime/loaded_resource_handle.h"
#include "comet/resource/runtime/resource_slot.h"

namespace comet {
namespace resource {
template <typename Tag, typename T>
class ResourceSlots {
 public:
  static_assert(std::is_base_of_v<Resource, T>, "T must derive from Resource");

  using Id = ResourceIdT<Tag>;
  using LoadedHandle = LoadedResourceHandle<Tag>;
  using Slot = ResourceSlot<Tag, T>;

  ResourceSlots() = default;

  explicit ResourceSlots(memory::Allocator* allocator,
                         usize initial_capacity = 0)
      : slots_{allocator},
        handles_{allocator, initial_capacity},
        allocator_{allocator} {}

  ResourceSlots(const ResourceSlots&) = delete;
  ResourceSlots(ResourceSlots&&) noexcept = default;
  ResourceSlots& operator=(const ResourceSlots&) = delete;
  ResourceSlots& operator=(ResourceSlots&&) noexcept = default;

  ~ResourceSlots() {
    COMET_ASSERT(!is_initialized_,
                 "Destructor called for resource slots, but it is still "
                 "initialized!");
  }

  void Initialize() {
    fiber::FiberLockGuard lock{mtx_};
    COMET_ASSERT(!is_initialized_,
                 "Tried to initialize resource slots, but it is already "
                 "done!");
    is_initialized_ = true;
  }

  void Destroy() noexcept {
    fiber::FiberLockGuard lock{mtx_};
    COMET_ASSERT(is_initialized_,
                 "Tried to destroy resource slots, but it is not "
                 "initialized!");
    is_initialized_ = false;
    slots_.Destroy();
    handles_.Destroy();
    pool_.Destroy();
  }

  LoadedHandle TryRetain(Id id, ResourceLifeSpan life_span) {
    fiber::FiberLockGuard lock{mtx_};

    COMET_ASSERT(is_initialized_,
                 "Tried to retain from resource slots, but it is not "
                 "initialized!");

    if (id.IsInvalid()) {
      return LoadedHandle::Invalid();
    }

    auto* handle_ptr{handles_.TryGet(
        internal::ResourceIdLifeSpanPair{id.GetValue(), life_span})};

    if (handle_ptr == nullptr) {
      return LoadedHandle::Invalid();
    }

    const auto handle{*handle_ptr};

    if (!pool_.IsAlive(handle)) {
      return LoadedHandle::Invalid();
    }

    const auto index{handle.GetIndex()};
    COMET_ASSERT(index < slots_.GetSize(),
                 "Loaded resource handle index is out of bounds!");

    auto& slot{slots_[index]};
    COMET_ASSERT(slot.resource != nullptr,
                 "Loaded resource slot contains a null resource!");
    ++slot.ref_count;
    return handle;
  }

  template <typename Loader, typename Releaser>
  LoadedHandle Acquire(Id id, ResourceLifeSpan life_span, Loader&& loader,
                       Releaser&& releaser) {
    COMET_ASSERT(id.IsValid(), "Tried to acquire an invalid resource ID!");

    const internal::ResourceIdLifeSpanPair key{id.GetValue(), life_span};

    {
      fiber::FiberLockGuard lock{mtx_};

      COMET_ASSERT(is_initialized_,
                   "Tried to acquire from resource slots, but it is not "
                   "initialized!");

      if (auto* handle_ptr{handles_.TryGet(key)}; handle_ptr != nullptr) {
        const auto handle{*handle_ptr};
        COMET_ASSERT(pool_.IsAlive(handle),
                     "Resource handle cache contains a dead loaded handle!");

        auto& slot{slots_[handle.GetIndex()]};
        COMET_ASSERT(slot.resource != nullptr,
                     "Loaded resource slot contains a null resource!");
        ++slot.ref_count;
        return handle;
      }
    }

    auto* resource{loader(id, life_span)};

    if (resource == nullptr) {
      return LoadedHandle::Invalid();
    }

    auto is_duplicate{false};
    auto handle{LoadedHandle::Invalid()};

    {
      fiber::FiberLockGuard lock{mtx_};

      COMET_ASSERT(is_initialized_,
                   "Tried to acquire from resource slots, but it is not "
                   "initialized!");

      if (auto* handle_ptr{handles_.TryGet(key)}; handle_ptr != nullptr) {
        handle = *handle_ptr;
        COMET_ASSERT(pool_.IsAlive(handle),
                     "Resource handle cache contains a dead loaded handle!");

        auto& slot{slots_[handle.GetIndex()]};
        COMET_ASSERT(slot.resource != nullptr,
                     "Loaded resource slot contains a null resource!");
        ++slot.ref_count;
        is_duplicate = true;
      } else {
        handle = pool_.Generate();
        const auto index{handle.GetIndex()};

        if (index >= slots_.GetSize()) {
          slots_.Resize(index + 1);
        }

        auto& slot{slots_[index]};
        slot.id = id;
        slot.life_span = life_span;
        slot.ref_count = 1;
        slot.resource = resource;

        handles_.Set(key, handle);
      }
    }

    if (is_duplicate) {
      releaser(resource);
    }

    return handle;
  }

  template <typename Releaser>
  void Release(LoadedHandle handle, Releaser&& releaser) {
    T* resource_to_release{nullptr};

    {
      fiber::FiberLockGuard lock{mtx_};

      COMET_ASSERT(is_initialized_,
                   "Tried to release from resource slots, but it is not "
                   "initialized!");

      if (!pool_.IsAlive(handle)) {
        return;
      }

      const auto index{handle.GetIndex()};
      COMET_ASSERT(index < slots_.GetSize(),
                   "Loaded resource handle index is out of bounds!");

      auto& slot{slots_[index]};
      COMET_ASSERT(slot.ref_count > 0,
                   "Tried to release a resource slot with ref count <= 0!");

      if (--slot.ref_count > 0) {
        return;
      }

      handles_.Remove(
          internal::ResourceIdLifeSpanPair{slot.id.GetValue(), slot.life_span});

      resource_to_release = slot.resource;
      slot.id.Invalidate();
      slot.life_span = ResourceLifeSpan::Unknown;
      slot.ref_count = 0;
      slot.resource = nullptr;

      pool_.Destroy(handle);
    }

    if (resource_to_release != nullptr) {
      releaser(resource_to_release);
    }
  }

  template <typename Releaser>
  void ReleaseAll(ResourceLifeSpan life_span, Releaser&& releaser) {
    Array<LoadedHandle> handles_to_release{allocator_};

    {
      fiber::FiberLockGuard lock{mtx_};

      COMET_ASSERT(is_initialized_,
                   "Tried to release all from resource slots, but it is not "
                   "initialized!");

      for (const auto& pair : handles_) {
        if (pair.key.life_span == life_span) {
          handles_to_release.PushBack(pair.value);
        }
      }
    }

    for (const auto& handle : handles_to_release) {
      Release(handle, std::forward<Releaser>(releaser));
    }
  }

  T* Get(LoadedHandle handle) {
    fiber::FiberLockGuard lock{mtx_};

    COMET_ASSERT(is_initialized_,
                 "Tried to get from resource slots, but it is not "
                 "initialized!");

    if (!pool_.IsAlive(handle)) {
      return nullptr;
    }

    const auto index{handle.GetIndex()};
    COMET_ASSERT(index < slots_.GetSize(),
                 "Loaded resource handle index is out of bounds!");
    return slots_[index].resource;
  }

  const T* Get(LoadedHandle handle) const {
    fiber::FiberLockGuard lock{mtx_};

    COMET_ASSERT(is_initialized_,
                 "Tried to get from resource slots, but it is not "
                 "initialized!");

    if (!pool_.IsAlive(handle)) {
      return nullptr;
    }

    const auto index{handle.GetIndex()};
    COMET_ASSERT(index < slots_.GetSize(),
                 "Loaded resource handle index is out of bounds!");
    return slots_[index].resource;
  }

  T* Get(Id id, ResourceLifeSpan life_span) {
    const auto handle{TryGetHandle(id, life_span)};

    if (!handle) {
      return nullptr;
    }

    return Get(handle);
  }

  const T* Get(Id id, ResourceLifeSpan life_span) const {
    const auto handle{TryGetHandle(id, life_span)};

    if (!handle) {
      return nullptr;
    }

    return Get(handle);
  }

  LoadedHandle TryGetHandle(Id id, ResourceLifeSpan life_span) const {
    fiber::FiberLockGuard lock{mtx_};

    COMET_ASSERT(is_initialized_,
                 "Tried to get handle from resource slots, but it is not "
                 "initialized!");

    if (id.IsInvalid()) {
      return LoadedHandle::Invalid();
    }

    const auto* handle_ptr{handles_.TryGet(
        internal::ResourceIdLifeSpanPair{id.GetValue(), life_span})};

    if (handle_ptr == nullptr) {
      return LoadedHandle::Invalid();
    }

    const auto handle{*handle_ptr};

    if (!pool_.IsAlive(handle)) {
      return LoadedHandle::Invalid();
    }

    return handle;
  }

  LoadedHandle RegisterImmortal(Id id, T* resource) {
    COMET_ASSERT(id.IsValid(),
                 "Tried to register an invalid immortal resource!");
    COMET_ASSERT(resource != nullptr, "Immortal resource is null!");

    const internal::ResourceIdLifeSpanPair key{id.GetValue(),
                                               ResourceLifeSpan::Immortal};

    fiber::FiberLockGuard lock{mtx_};

    COMET_ASSERT(is_initialized_,
                 "Tried to register immortal resource into resource slots, "
                 "but it is not initialized!");

    if (auto* handle_ptr{handles_.TryGet(key)}; handle_ptr != nullptr) {
      const auto handle{*handle_ptr};

      if (pool_.IsAlive(handle)) {
        COMET_ASSERT(slots_[handle.GetIndex()].resource != nullptr,
                     "Immortal resource slot contains a null resource!");
        return handle;
      }

      handles_.Remove(key);
    }

    const auto handle{pool_.Generate()};
    const auto index{handle.GetIndex()};

    if (index >= slots_.GetSize()) {
      slots_.Resize(index + 1);
    }

    auto& slot{slots_[index]};
    slot.id = id;
    slot.life_span = ResourceLifeSpan::Immortal;
    slot.ref_count = kUSizeMax;
    slot.resource = resource;

    handles_.Set(key, handle);
    return handle;
  }

  bool IsAlive(LoadedHandle handle) const noexcept {
    fiber::FiberLockGuard lock{mtx_};

    COMET_ASSERT(is_initialized_,
                 "Tried to query liveness from resource slots, but it is not "
                 "initialized!");

    return pool_.IsAlive(handle);
  }

  bool IsInitialized() const noexcept { return is_initialized_; }

 private:
  bool is_initialized_{false};
  mutable fiber::FiberMutex mtx_{};
  HandlePool<Tag> pool_{};
  Array<Slot> slots_{};
  Map<internal::ResourceIdLifeSpanPair, LoadedHandle> handles_{};
  memory::Allocator* allocator_{nullptr};
};
}  // namespace resource
}  // namespace comet

#endif  // COMET_COMET_RESOURCE_RUNTIME_RESOURCE_SLOTS_H_