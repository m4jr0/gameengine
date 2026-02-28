// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RESOURCE_HANDLER_RESOURCE_HANDLER_UTILS_H_
#define COMET_COMET_RESOURCE_HANDLER_RESOURCE_HANDLER_UTILS_H_

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

namespace comet {
namespace resource {
namespace internal {
template <typename T>
class DefaultResources {
 public:
  static_assert(std::is_base_of_v<Resource, T>, "T must derive from Resource");
  inline static constexpr ResourceId kFallbackResourceId_{0};

  DefaultResources() = default;
  DefaultResources(memory::Allocator* allocator);
  DefaultResources(const DefaultResources&) = delete;
  DefaultResources(DefaultResources&&) = delete;
  DefaultResources& operator=(const DefaultResources&) = delete;
  DefaultResources& operator=(DefaultResources&&) = delete;
  virtual ~DefaultResources() = default;

  void Initialize();
  void Destroy();
  void Set(T* resource);
  T* TryGet(ResourceId id);
  bool IsDefault(ResourceId id);

  void Reserve(usize capacity);
  T* GetFallback();

 private:
  memory::Allocator* allocator_{nullptr};
  Map<ResourceId, T*> defaults_{};
};

template <typename T>
inline DefaultResources<T>::DefaultResources(memory::Allocator* allocator)
    : allocator_{allocator} {}

template <typename T>
inline void DefaultResources<T>::Initialize() {
  defaults_ = Map<ResourceId, T*>{allocator_};
}

template <typename T>
inline void DefaultResources<T>::Destroy() {
  defaults_.Destroy();
}

template <typename T>
inline void DefaultResources<T>::Set(T* resource) {
  defaults_.Emplace(resource->id, resource);
}

template <typename T>
inline T* DefaultResources<T>::TryGet(ResourceId id) {
  auto** resource_ptr{defaults_.TryGet(id)};
  return resource_ptr == nullptr ? nullptr : *resource_ptr;
}

template <typename T>
inline bool DefaultResources<T>::IsDefault(ResourceId id) {
  return defaults_.IsContained(id);
}

template <typename T>
inline void DefaultResources<T>::Reserve(usize capacity) {
  defaults_.Reserve(capacity);
}

template <typename T>
inline T* DefaultResources<T>::GetFallback() {
  return TryGet(kFallbackResourceId_);
}

struct ResourceIdLifeSpanPair {
  ResourceId id{kInvalidResourceId};
  ResourceLifeSpan life_span{ResourceLifeSpan::Unknown};

  bool operator==(const ResourceIdLifeSpanPair& other) const;
};

HashValue GenerateHash(const ResourceIdLifeSpanPair& value);

template <typename T>
class ResourceCache {
 public:
  static_assert(std::is_base_of_v<Resource, T>, "T must derive from Resource");
  ResourceCache() = default;
  ResourceCache(memory::Allocator* allocator, usize initial_capacity = 0);
  ResourceCache(const ResourceCache&) = delete;
  ResourceCache(ResourceCache&&) = delete;
  ResourceCache& operator=(const ResourceCache&) = delete;
  ResourceCache& operator=(ResourceCache&&) = delete;
  virtual ~ResourceCache() = default;

  void Destroy();

  void Reserve(usize capacity);
  T* TryGet(ResourceId id, ResourceLifeSpan life_span);
  void Set(ResourceId id, ResourceLifeSpan life_span, T* resource);
  void Unset(ResourceId id, ResourceLifeSpan life_span);
  void UnsetAll(ResourceLifeSpan life_span);

 private:
  fiber::FiberMutex mtx_{};
  Map<ResourceIdLifeSpanPair, T*> resources_{};
};

template <typename T>
inline ResourceCache<T>::ResourceCache(memory::Allocator* allocator,
                                       usize initial_capacity)
    : resources_{allocator, initial_capacity} {}

template <typename T>
inline void ResourceCache<T>::Destroy() {
  resources_.Destroy();
}

template <typename T>
inline void ResourceCache<T>::Reserve(usize capacity) {
  resources_.Reserve(capacity);
}

template <typename T>
inline T* ResourceCache<T>::TryGet(ResourceId id, ResourceLifeSpan life_span) {
  fiber::FiberLockGuard lock{mtx_};
  auto** resource_ptr{resources_.TryGet(ResourceIdLifeSpanPair{id, life_span})};

  if (resource_ptr == nullptr) {
    return nullptr;
  }

  return *resource_ptr;
}

template <typename T>
inline void ResourceCache<T>::Set(ResourceId id, ResourceLifeSpan life_span,
                                  T* resource) {
  fiber::FiberLockGuard lock{mtx_};
  resources_.Set(ResourceIdLifeSpanPair{id, life_span}, resource);
}

template <typename T>
inline void ResourceCache<T>::Unset(ResourceId id, ResourceLifeSpan life_span) {
  fiber::FiberLockGuard lock{mtx_};
  resources_.Remove(ResourceIdLifeSpanPair{id, life_span});
}

template <typename T>
inline void ResourceCache<T>::UnsetAll(ResourceLifeSpan life_span) {
  fiber::FiberLockGuard lock{mtx_};

  resources_.RemoveIf([=](const ResourceIdLifeSpanPair& key, const T*) {
    return key.life_span == life_span;
  });
}

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
  virtual ~LoadingTracker() = default;

  void Initialize();
  void Destroy();

  LoadingResourceState<T>* RequestLoad(ResourceId id,
                                       ResourceLifeSpan life_span,
                                       bool& is_already_loading);
  T* Wait(LoadingResourceState<T>* state);
  void Finish(LoadingResourceState<T>* state, T* resource);

 private:
  void ReleaseLoadingState(LoadingResourceState<T>* state);

  fiber::FiberMutex mtx_{};
  Map<ResourceIdLifeSpanPair, LoadingResourceState<T>*> loading_{};
  memory::FiberFreeListAllocator state_allocator_{
      sizeof(LoadingResourceState<T>), 128, memory::kEngineMemoryTagResource};
  memory::Allocator* ptr_allocator_{nullptr};
};

template <typename T>
inline LoadingTracker<T>::LoadingTracker(memory::Allocator* ptr_allocator)
    : ptr_allocator_{ptr_allocator} {}

template <typename T>
inline void LoadingTracker<T>::Initialize() {
  loading_ =
      Map<ResourceIdLifeSpanPair, LoadingResourceState<T>*>{ptr_allocator_};
}

template <typename T>
inline void LoadingTracker<T>::Destroy() {
  loading_.Destroy();
}

template <typename T>
inline LoadingResourceState<T>* LoadingTracker<T>::RequestLoad(
    ResourceId id, ResourceLifeSpan life_span, bool& is_already_loading) {
  COMET_PROFILE("LoadingTracker<T>::RequestLoad");
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
  state->is_loading = true;
  state->key = key;
  state->ref_count = 1;
  return loading_.Emplace(key, state).value;
}

template <typename T>
inline T* LoadingTracker<T>::Wait(LoadingResourceState<T>* state) {
  COMET_PROFILE("LoadingTracker<T>::Wait");

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
  fiber::FiberLockGuard lock{mtx_};
  state->resource = resource;
  state->is_loading = false;
  ReleaseLoadingState(state);
}

template <typename T>
inline void LoadingTracker<T>::ReleaseLoadingState(
    LoadingResourceState<T>* state) {
  if (--state->ref_count == 0) {
    loading_.Remove(state->key);
    state_allocator_.Deallocate(state);
  }
}

struct LifeSpanAllocators {
  memory::Allocator* scene{nullptr};
  memory::Allocator* global{nullptr};
};

TString& GenerateTlsResourceAbsPath(CTStringView root_resource_path,
                                    ResourceId resource_id);
}  // namespace internal
}  // namespace resource
}  // namespace comet

#endif  // COMET_COMET_RESOURCE_HANDLER_RESOURCE_HANDLER_UTILS_H_
