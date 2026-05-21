// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_RUNTIME_RESOURCE_RESOURCE_HANDLER_UTILS_H_
#define COMET_RUNTIME_RESOURCE_RESOURCE_HANDLER_UTILS_H_

// External. ///////////////////////////////////////////////////////////////////
#include <type_traits>
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/concurrency/fiber/fiber_primitive.h"
#include "comet/core/essentials.h"
#include "comet/core/hash.h"
#include "comet/core/memory/allocator/allocator.h"
#include "comet/core/memory/allocator/free_list_allocator.h"
#include "comet/core/memory/memory.h"
#include "comet/core/type/map.h"
#include "comet/core/type/tstring.h"
#include "comet/profiler/profiler.h"
#include "comet/resource/resource.h"
#include "comet/resource/type/common.h"

namespace comet {
namespace resource {
namespace internal {
template <typename T>
class DefaultResources {
 public:
  static_assert(std::is_base_of_v<Resource, T>, "T must derive from Resource");

  DefaultResources() = default;
  DefaultResources(memory::Allocator* allocator);
  DefaultResources(const DefaultResources&) = delete;
  DefaultResources(DefaultResources&&) = delete;
  DefaultResources& operator=(const DefaultResources&) = delete;
  DefaultResources& operator=(DefaultResources&&) = delete;
  ~DefaultResources();

  void Initialize();
  void Destroy();

  void Set(T* resource);

  T* TryGet(RawResourceId id);
  const T* TryGet(RawResourceId id) const;

  bool IsDefault(RawResourceId id) const;

  void Reserve(usize capacity);

  T* GetFallback();
  const T* GetFallback() const;

  usize GetEntryCount() const noexcept;
  bool IsEmpty() const noexcept;

 private:
  bool is_initialized_{false};
  memory::Allocator* allocator_{nullptr};
  Map<RawResourceId, T*> defaults_{};
};

template <typename T>
inline DefaultResources<T>::DefaultResources(memory::Allocator* allocator)
    : allocator_{allocator} {}

template <typename T>
inline DefaultResources<T>::~DefaultResources() {
  COMET_ASSERT(!is_initialized_, "DefaultResources<T>::~DefaultResources",
               "default resources are still initialized");
}

template <typename T>
inline void DefaultResources<T>::Initialize() {
  COMET_ASSERT(!is_initialized_, "DefaultResources<T>::Initialize",
               "default resources are already initialized");
  COMET_ASSERT(allocator_ != nullptr, "DefaultResources<T>::Initialize",
               "allocator is null");
  defaults_ = Map<RawResourceId, T*>{allocator_};

  is_initialized_ = true;
}

template <typename T>
inline void DefaultResources<T>::Destroy() {
  COMET_ASSERT(is_initialized_, "DefaultResources<T>::Destroy",
               "default resources are not initialized");
  defaults_.Release();

  is_initialized_ = false;
}

template <typename T>
inline void DefaultResources<T>::Set(T* resource) {
  COMET_ASSERT(is_initialized_, "DefaultResources<T>::Set",
               "default resources are not initialized");
  COMET_ASSERT(resource != nullptr, "DefaultResources<T>::Set",
               "default resource is null");
  COMET_ASSERT(resource->id != kInvalidRawResourceId,
               "DefaultResources<T>::Set", "default resource id is invalid");
  COMET_ASSERT(!defaults_.IsContained(resource->id), "DefaultResources<T>::Set",
               "default resource is already registered", "resource_id",
               resource->id);
  defaults_.Emplace(resource->id, resource);
}

template <typename T>
inline T* DefaultResources<T>::TryGet(RawResourceId id) {
  COMET_ASSERT(is_initialized_, "DefaultResources<T>::TryGet",
               "default resources are not initialized");
  auto** resource_ptr{defaults_.TryGet(id)};
  return resource_ptr == nullptr ? nullptr : *resource_ptr;
}

template <typename T>
inline const T* DefaultResources<T>::TryGet(RawResourceId id) const {
  COMET_ASSERT(is_initialized_, "DefaultResources<T>::TryGet",
               "default resources are not initialized");
  T* const* resource_ptr{defaults_.TryGet(id)};
  return resource_ptr == nullptr ? nullptr : *resource_ptr;
}

template <typename T>
inline bool DefaultResources<T>::IsDefault(RawResourceId id) const {
  COMET_ASSERT(is_initialized_, "DefaultResources<T>::IsDefault",
               "default resources are not initialized");
  return defaults_.IsContained(id);
}

template <typename T>
inline void DefaultResources<T>::Reserve(usize capacity) {
  COMET_ASSERT(is_initialized_, "DefaultResources<T>::Reserve",
               "default resources are not initialized");
  defaults_.Reserve(capacity);
}

template <typename T>
inline const T* DefaultResources<T>::GetFallback() const {
  COMET_ASSERT(is_initialized_, "DefaultResources<T>::GetFallback",
               "default resources are not initialized");
  return TryGet(kFallbackRawResourceId);
}

template <typename T>
inline T* DefaultResources<T>::GetFallback() {
  COMET_ASSERT(is_initialized_, "DefaultResources<T>::GetFallback",
               "default resources are not initialized");
  return TryGet(kFallbackRawResourceId);
}

template <typename T>
inline usize DefaultResources<T>::GetEntryCount() const noexcept {
  return defaults_.GetEntryCount();
}

template <typename T>
inline bool DefaultResources<T>::IsEmpty() const noexcept {
  return defaults_.IsEmpty();
}

struct ResourceIdLifeSpanPair {
  RawResourceId id{kInvalidRawResourceId};
  ResourceLifeSpan life_span{ResourceLifeSpan::Unknown};

  bool operator==(const ResourceIdLifeSpanPair& other) const;
};

HashValue GenerateHash(const ResourceIdLifeSpanPair& value);

template <typename T>
struct LoadingResourceState {
  static_assert(std::is_base_of_v<Resource, T>, "T must derive from Resource");
  bool is_loading{false};
  ResourceIdLifeSpanPair key{};
  usize ref_count{0};
  T* resource{nullptr};
};

template <typename T>
class LoadingTracker {
 public:
  static_assert(std::is_base_of_v<Resource, T>, "T must derive from Resource");
  LoadingTracker() = default;
  LoadingTracker(memory::Allocator* ptr_allocator);
  LoadingTracker(const LoadingTracker&) = delete;
  LoadingTracker(LoadingTracker&&) = delete;
  LoadingTracker& operator=(const LoadingTracker&) = delete;
  LoadingTracker& operator=(LoadingTracker&&) = delete;
  ~LoadingTracker();

  void Initialize();
  void Destroy();

  LoadingResourceState<T>* RequestLoad(RawResourceId id,
                                       ResourceLifeSpan life_span,
                                       bool& is_already_loading);
  T* Wait(LoadingResourceState<T>* state);
  void Finish(LoadingResourceState<T>* state, T* resource);
  void Release(LoadingResourceState<T>* state);

 private:
  void ReleaseLoadingState(LoadingResourceState<T>* state);

  bool is_initialized_{false};
  mutable fiber::FiberMutex mtx_{};
  Map<ResourceIdLifeSpanPair, LoadingResourceState<T>*> loading_{};
  memory::FiberFreeListAllocator state_allocator_{
      sizeof(LoadingResourceState<T>), 128, memory::kEngineMemoryTagResource};
  memory::Allocator* ptr_allocator_{nullptr};
};

template <typename T>
inline LoadingTracker<T>::LoadingTracker(memory::Allocator* ptr_allocator)
    : ptr_allocator_{ptr_allocator} {
  COMET_ASSERT(ptr_allocator_ != nullptr,
               "resource::internal::LoadingTracker<T>::Initialize",
               "pointer allocator is null");
}

template <typename T>
inline LoadingTracker<T>::~LoadingTracker() {
  COMET_ASSERT(!is_initialized_, "LoadingTracker<T>::~LoadingTracker",
               "loading tracker is still initialized");
}

template <typename T>
inline void LoadingTracker<T>::Initialize() {
  COMET_ASSERT(!is_initialized_, "LoadingTracker<T>::Initialize",
               "loading tracker is already initialized");
  loading_ =
      Map<ResourceIdLifeSpanPair, LoadingResourceState<T>*>{ptr_allocator_};
  state_allocator_.Initialize();

  is_initialized_ = true;
}

template <typename T>
inline void LoadingTracker<T>::Destroy() {
  COMET_ASSERT(is_initialized_, "LoadingTracker<T>::Destroy",
               "loading tracker is not initialized");
  COMET_ASSERT(loading_.IsEmpty(),
               "resource::internal::LoadingTracker<T>::Destroy",
               "loading tracker destroyed with live states", "remaining_states",
               loading_.GetEntryCount());

  loading_.Release();
  state_allocator_.Destroy();

  is_initialized_ = false;
}

template <typename T>
inline LoadingResourceState<T>* LoadingTracker<T>::RequestLoad(
    RawResourceId id, ResourceLifeSpan life_span, bool& is_already_loading) {
  COMET_PROFILE("LoadingTracker<T>::RequestLoad");
  COMET_ASSERT(is_initialized_, "LoadingTracker<T>::RequestLoad",
               "loading tracker is not initialized");

  COMET_ASSERT(id != kInvalidRawResourceId,
               "resource::internal::LoadingTracker<T>::RequestLoad",
               "resource id is invalid");
  COMET_ASSERT(life_span != ResourceLifeSpan::Unknown,
               "resource::internal::LoadingTracker<T>::RequestLoad",
               "resource life span is unknown");

  ResourceIdLifeSpanPair key{id, life_span};
  fiber::FiberLockGuard lock{mtx_};
  auto** state_ptr{loading_.TryGet(key)};
  is_already_loading = state_ptr != nullptr;
  LoadingResourceState<T>* state;

  if (is_already_loading) {
    state = *state_ptr;
    ++state->ref_count;
    return state;
  }

  state = state_allocator_
              .AllocateOneAndPopulate<internal::LoadingResourceState<T>>();
  COMET_ASSERT(state != nullptr,
               "resource::internal::LoadingTracker<T>::RequestLoad",
               "failed to allocate loading state");

  state->is_loading = true;
  state->key = key;
  state->ref_count = 1;

  auto inserted{loading_.Emplace(key, state)};
  COMET_ASSERT(inserted.value != nullptr,
               "resource::internal::LoadingTracker<T>::RequestLoad",
               "failed to register loading state", "resource_id", id);

  return inserted.value;
}

template <typename T>
inline T* LoadingTracker<T>::Wait(LoadingResourceState<T>* state) {
  COMET_PROFILE("LoadingTracker<T>::Wait");
  COMET_ASSERT(is_initialized_, "LoadingTracker<T>::Wait",
               "loading tracker is not initialized");
  COMET_ASSERT(state != nullptr, "resource::internal::LoadingTracker<T>::Wait",
               "loading state is null");

  for (;;) {
    {
      fiber::FiberLockGuard lock{mtx_};

      if (!state->is_loading) {
        auto* resource{state->resource};
        ReleaseLoadingState(state);
        return resource;
      }
    }

    fiber::Yield();
  }
}

template <typename T>
inline void LoadingTracker<T>::Finish(LoadingResourceState<T>* state,
                                      T* resource) {
  COMET_PROFILE("LoadingTracker<T>::Finish");
  COMET_ASSERT(is_initialized_, "LoadingTracker<T>::Finish",
               "loading tracker is not initialized");
  COMET_ASSERT(state != nullptr,
               "resource::internal::LoadingTracker<T>::Finish",
               "loading state is null");
  COMET_ASSERT(state->is_loading,
               "resource::internal::LoadingTracker<T>::Finish",
               "loading state is already finished");

  fiber::FiberLockGuard lock{mtx_};
  state->resource = resource;
  state->is_loading = false;
}

template <typename T>
inline void LoadingTracker<T>::Release(LoadingResourceState<T>* state) {
  COMET_ASSERT(state != nullptr,
               "resource::internal::LoadingTracker<T>::Release",
               "loading state is null");
  COMET_ASSERT(is_initialized_, "LoadingTracker<T>::Release",
               "loading tracker is not initialized");

  fiber::FiberLockGuard lock{mtx_};
  ReleaseLoadingState(state);
}

template <typename T>
inline void LoadingTracker<T>::ReleaseLoadingState(
    LoadingResourceState<T>* state) {
  COMET_ASSERT(state != nullptr,
               "resource::internal::LoadingTracker<T>::ReleaseLoadingState",
               "loading state is null");
  COMET_ASSERT(state->ref_count > 0,
               "resource::internal::LoadingTracker<T>::ReleaseLoadingState",
               "loading state reference count is already zero");
  COMET_ASSERT(state->key.id != kInvalidRawResourceId,
               "resource::internal::LoadingTracker<T>::ReleaseLoadingState",
               "loading state resource id is invalid");

  if (--state->ref_count == 0) {
    loading_.Remove(state->key);
    state_allocator_.Deallocate(state);
  }
}

struct LifeSpanAllocators {
  memory::Allocator* scene{nullptr};
  memory::Allocator* global{nullptr};
  memory::Allocator* immortal{nullptr};
};

TString& GenerateTlsResourceAbsPath(CTStringView root_resource_path,
                                    RawResourceId resource_id);
}  // namespace internal
}  // namespace resource
}  // namespace comet

#endif  // COMET_RUNTIME_RESOURCE_RESOURCE_HANDLER_UTILS_H_
