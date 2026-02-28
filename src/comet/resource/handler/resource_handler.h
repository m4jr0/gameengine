// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RESOURCE_RESOURCE_HANDLER_H_
#define COMET_COMET_RESOURCE_RESOURCE_HANDLER_H_

// External. ///////////////////////////////////////////////////////////////////
#include <type_traits>
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/essentials.h"
#include "comet/core/frame/frame_event.h"
#include "comet/core/logger.h"
#include "comet/core/memory/allocator/allocator.h"
#include "comet/core/memory/allocator/free_list_allocator.h"
#include "comet/core/memory/memory.h"
#include "comet/core/type/hash_set.h"
#include "comet/core/type/string_id.h"
#include "comet/core/type/tstring.h"
#include "comet/event/event.h"
#include "comet/event/event_manager.h"
#include "comet/profiler/profiler.h"
#include "comet/resource/handler/resource_handler_utils.h"
#include "comet/resource/resource.h"
#include "comet/scene/scene_event.h"

namespace comet {
namespace resource {
struct ResourceHandlerDescr {
  CTStringView root_path{};
  usize initial_capacity{kInvalidSize};
  internal::LifeSpanAllocators life_span_allocators{};
  memory::Allocator* ptr_allocator{nullptr};
  memory::Allocator* byte_allocator{nullptr};
};

template <typename T>
class ResourceHandler {
 public:
  static_assert(std::is_base_of_v<Resource, T>, "T must derive from Resource");
  ResourceHandler(const ResourceHandlerDescr& descr);
  ResourceHandler(const ResourceHandler&) = delete;
  ResourceHandler(ResourceHandler&&) = delete;
  ResourceHandler& operator=(const ResourceHandler&) = delete;
  ResourceHandler& operator=(ResourceHandler&&) = delete;
  virtual ~ResourceHandler();

  virtual void Initialize();
  virtual void Destroy();

  T* Load(CTStringView path,
          ResourceLifeSpan life_span = ResourceLifeSpan::Manual);
  T* Load(ResourceId id, ResourceLifeSpan life_span = ResourceLifeSpan::Manual);
  void Unload(CTStringView path);
  void Unload(ResourceId id);

  virtual ResourceFile Pack(const T& resource,
                            CompressionMode compression_mode) = 0;
  virtual void Unpack(const ResourceFile& file, ResourceLifeSpan life_span,
                      T* resource) = 0;

 protected:
  inline static const usize kDefaultResourceCapacity_{128};

  virtual void InitializeDefaults() {};
  virtual void DestroyDefaults() {};
  T* LoadInternal(ResourceId id, ResourceLifeSpan life_span);
  void DestroyDeleted();
  memory::Allocator* ResolveAllocator(memory::Allocator* default_allocator,
                                      ResourceLifeSpan life_span) const;
#ifdef COMET_PROFILING
  void SetupProfiling(const schar* method_name, ResourceId resource_id) const;
#define COMET_RESOURCE_HANDLER_SETUP_PROFILING(method_name, resource_id) \
  SetupProfiling(method_name, resource_id)
#else
#define COMET_RESOURCE_HANDLER_SETUP_PROFILING(method_name, resource_id)
#endif  // COMET_PROFILING

  void OnEvent(const event::Event& event);

  CTStringView root_path_{};
  internal::DefaultResources<T> defaults_{};
  internal::ResourceCache<T> cache_{};
  internal::LoadingTracker<T> tracker_{};
  memory::FiberFreeListAllocator resource_allocator_{};
  internal::LifeSpanAllocators life_span_allocators_{};
  HashSet<T*> deleted_resources_{};
  memory::Allocator* byte_allocator_{nullptr};

 private:
  bool is_initialized_{false};
};

template <typename T>
inline ResourceHandler<T>::ResourceHandler(const ResourceHandlerDescr& descr)
    : root_path_{descr.root_path},
      defaults_{descr.ptr_allocator},
      cache_{descr.ptr_allocator, descr.initial_capacity != kInvalidSize
                                      ? descr.initial_capacity
                                      : kDefaultResourceCapacity_},
      tracker_{descr.ptr_allocator},
      resource_allocator_{sizeof(T),
                          descr.initial_capacity != kInvalidSize
                              ? descr.initial_capacity
                              : kDefaultResourceCapacity_,
                          memory::kEngineMemoryTagResource},
      life_span_allocators_{descr.life_span_allocators},
      byte_allocator_{descr.byte_allocator} {}

template <typename T>
inline ResourceHandler<T>::~ResourceHandler() {
  COMET_ASSERT(!is_initialized_,
               "Destructor called for resource handler, but it is "
               "still initialized!");
}

template <typename T>
inline void ResourceHandler<T>::Initialize() {
  COMET_ASSERT(!is_initialized_,
               "Tried to initialize resource handler, but it is already done!");

  defaults_.Initialize();
  tracker_.Initialize();
  deleted_resources_ = HashSet<T*>{byte_allocator_, 64};
  InitializeDefaults();

  const auto event_function{
      COMET_EVENT_BIND_FUNCTION(ResourceHandler<T>::OnEvent)};
  event::EventManager::Get().Register(event_function,
                                      scene::SceneUnloadedEvent::kStaticType_);
  event::EventManager::Get().Register(event_function,
                                      frame::NewFrameEvent::kStaticType_);

  is_initialized_ = true;
}

template <typename T>
inline void ResourceHandler<T>::Destroy() {
  COMET_ASSERT(is_initialized_,
               "Tried to destroy resource handler, but it is not initialized!");

  defaults_.Destroy();
  tracker_.Destroy();
  cache_.Destroy();
  DestroyDefaults();
  is_initialized_ = false;
}

template <typename T>
inline T* ResourceHandler<T>::Load(CTStringView path,
                                   ResourceLifeSpan life_span) {
  COMET_PROFILE("LoadingTracker<T>::Finish");
  return Load(GenerateResourceIdFromPath<T>(path), life_span);
}

template <typename T>
inline T* ResourceHandler<T>::Load(ResourceId id, ResourceLifeSpan life_span) {
  COMET_RESOURCE_HANDLER_SETUP_PROFILING("Load", id);
  T* resource;

  if ((resource = defaults_.TryGet(id)) != nullptr) {
    return resource;
  }

  if ((resource = cache_.TryGet(id, life_span)) != nullptr) {
    return resource;
  }

  auto is_already_loading{false};
  auto* loading_state{tracker_.RequestLoad(id, life_span, is_already_loading)};

  if (is_already_loading) {
    return tracker_.Wait(loading_state);
  }

  resource = LoadInternal(id, life_span);
  tracker_.Finish(loading_state, resource);
  return resource;
}

template <typename T>
inline void ResourceHandler<T>::Unload(CTStringView path) {
  Unload(GenerateResourceIdFromPath<T>(path));
}

template <typename T>
inline void ResourceHandler<T>::Unload(ResourceId id) {
  COMET_RESOURCE_HANDLER_SETUP_PROFILING("Unload", id);
  auto* resource{cache_.TryGet(id, ResourceLifeSpan::Manual)};
  COMET_ASSERT(resource != nullptr, "Tried to unload resource that is null!");

  // Case: default resources. They cannot be unloaded.
  if (defaults_.IsDefault(resource->id)) {
    return;
  }

  COMET_ASSERT(resource->ref_count > 0, "Resource with ID ",
               COMET_STRING_ID_LABEL(resource->id),
               ", was asked to be unloaded, but reference count is already 0!");

  // Case: resource is still used somewhere.
  if (--resource->ref_count >= 1) {
    return;
  }

  cache_.Unset(resource->id, ResourceLifeSpan::Manual);
  deleted_resources_.Add(resource);
}

template <typename T>
inline T* ResourceHandler<T>::LoadInternal(ResourceId id,
                                           ResourceLifeSpan life_span) {
  COMET_RESOURCE_HANDLER_SETUP_PROFILING("LoadInternal", id);
  ResourceFile file{};
  file.descr = Array<u8>{byte_allocator_};
  file.data = Array<u8>{byte_allocator_};
  const auto& resource_abs_path{
      internal::GenerateTlsResourceAbsPath(root_path_, id)};
  T* resource{nullptr};

  if (LoadResourceFile(resource_abs_path, file)) {
    resource = resource_allocator_.AllocateOneAndPopulate<T>();
    Unpack(file, life_span, resource);
    resource->ref_count = 1;
    cache_.Set(resource->id, life_span, resource);
  } else {
    COMET_LOG_RESOURCE_ERROR(
        "Unable to get resource with ID: ", COMET_STRING_ID_LABEL(id), ".");
    resource = defaults_.GetFallback();
  }

  return resource;
}

template <typename T>
inline void ResourceHandler<T>::DestroyDeleted() {
  for (auto* resource : deleted_resources_) {
    resource->~T();
    resource_allocator_.Deallocate(resource);
  }

  deleted_resources_.Clear();
}

template <typename T>
inline memory::Allocator* ResourceHandler<T>::ResolveAllocator(
    memory::Allocator* default_allocator, ResourceLifeSpan life_span) const {
  switch (life_span) {
    case ResourceLifeSpan::Scene:
      return life_span_allocators_.scene;

    case ResourceLifeSpan::Global:
      return life_span_allocators_.global;

    case ResourceLifeSpan::Manual:
      return default_allocator;

    default:
      COMET_ASSERT(
          false, "Unknown or unsupported lock type provided: ",
          static_cast<std::underlying_type_t<ResourceLifeSpan>>(life_span),
          "!");
      return nullptr;
  }
}

template <typename T>
inline void ResourceHandler<T>::OnEvent(const event::Event& event) {
  const auto& event_type{event.GetType()};

  if (event_type == scene::SceneUnloadedEvent::kStaticType_) {
    cache_.UnsetAll(ResourceLifeSpan::Scene);
  } else if (event_type == frame::NewFrameEvent::kStaticType_) {
    DestroyDeleted();
  }
}

#ifdef COMET_PROFILING
template <typename T>
inline void ResourceHandler<T>::SetupProfiling(const schar* method_name,
                                               ResourceId resource_id) const {
  schar label[profiler::kMaxProfileLabelLen + 1]{'\0'};

  constexpr schar kLabelPrefix[]{"ResourceHandler<T>::"};
  constexpr auto kLabelPrefixLen{GetLength(kLabelPrefix)};
  constexpr schar kLabelOpen[]{" ("};
  constexpr auto kLabelOpenLen{GetLength(kLabelOpen)};
  constexpr schar kLabelClose[]{")"};
  constexpr auto kLabelCloseLen{GetLength(kLabelClose)};

  usize offset{0};

  Copy(label + offset, kLabelPrefix, kLabelPrefixLen);
  offset += kLabelPrefixLen;

  const auto method_len{GetLength(method_name)};
  Copy(label + offset, method_name, method_len);
  offset += method_len;

  Copy(label + offset, kLabelOpen, kLabelOpenLen);
  offset += kLabelOpenLen;

  usize id_len{0};
  ConvertToStr(resource_id, label + offset,
               profiler::kMaxProfileLabelLen - offset, &id_len);
  offset += id_len;

  Copy(label + offset, kLabelClose, kLabelCloseLen);
  offset += kLabelCloseLen;

  label[offset] = '\0';
  COMET_PROFILE(label);
}
#endif  // COMET_PROFILING
}  // namespace resource
}  // namespace comet

#endif  // COMET_COMET_RESOURCE_RESOURCE_HANDLER_H_
