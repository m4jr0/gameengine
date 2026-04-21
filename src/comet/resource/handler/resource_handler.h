// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RESOURCE_HANDLER_RESOURCE_HANDLER_H_
#define COMET_COMET_RESOURCE_HANDLER_RESOURCE_HANDLER_H_

// External. ///////////////////////////////////////////////////////////////////
#include <type_traits>
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/essentials.h"
#include "comet/core/frame/frame_event.h"
#include "comet/core/logger/logging.h"
#include "comet/core/memory/allocator/allocator.h"
#include "comet/core/memory/allocator/free_list_allocator.h"
#include "comet/core/memory/memory.h"
#include "comet/core/type/hash_set.h"
#include "comet/core/type/tstring.h"
#include "comet/core/type_trait.h"
#include "comet/event/event.h"
#include "comet/event/event_manager.h"
#include "comet/profiler/profiler.h"
#include "comet/resource/handler/resource_handler_utils.h"
#include "comet/resource/label/resource_common_label.h"
#include "comet/resource/resource.h"
#include "comet/resource/resource_id.h"
#include "comet/resource/runtime/loaded_resource_handle.h"
#include "comet/resource/runtime/resource_slots.h"
#include "comet/resource/type/resource_common_type.h"
#include "comet/scene/scene_event.h"

namespace comet {
namespace resource {
namespace internal {
template <typename Handler>
class LoadedResourceScope {
 public:
  using LoadedHandle = typename Handler::LoadedHandle;

  LoadedResourceScope(Handler& handler, LoadedHandle handle) noexcept
      : handler_{&handler}, handle_{handle} {}

  LoadedResourceScope(const LoadedResourceScope&) = delete;
  LoadedResourceScope(LoadedResourceScope&& other) noexcept
      : handler_{other.handler_}, handle_{other.handle_} {
    other.handler_ = nullptr;
    other.handle_ = LoadedHandle::Invalid();
  }

  LoadedResourceScope& operator=(const LoadedResourceScope&) = delete;
  LoadedResourceScope& operator=(LoadedResourceScope&&) = delete;

  ~LoadedResourceScope() {
    if (handler_ != nullptr && handle_) {
      handler_->Unload(handle_);
    }
  }

 private:
  Handler* handler_{nullptr};
  LoadedHandle handle_{};
};
}  // namespace internal

struct ResourceHandlerDescr {
  memory::MemoryTag memory_tag{memory::kEngineMemoryTagUntagged};
  CTStringView root_path{};
  usize initial_capacity{kInvalidSize};
  internal::LifeSpanAllocators life_span_allocators{};
  memory::Allocator* ptr_allocator{nullptr};
  memory::Allocator* byte_allocator{nullptr};
};

template <typename Tag, typename T>
class ResourceHandler {
 public:
  static_assert(std::is_base_of_v<Resource, T>, "T must derive from Resource");

  using Id = ResourceIdT<Tag>;
  using LoadedHandle = LoadedResourceHandle<Tag>;

  explicit ResourceHandler(const ResourceHandlerDescr& descr);
  ResourceHandler(const ResourceHandler&) = delete;
  ResourceHandler(ResourceHandler&&) = delete;
  ResourceHandler& operator=(const ResourceHandler&) = delete;
  ResourceHandler& operator=(ResourceHandler&&) = delete;
  virtual ~ResourceHandler();

  void Initialize();
  void Destroy();

  T* Get(LoadedHandle handle);
  const T* Get(LoadedHandle handle) const;

  LoadedHandle Load(CTStringView path,
                    ResourceLifeSpan life_span = ResourceLifeSpan::Manual);
  LoadedHandle Load(Id id,
                    ResourceLifeSpan life_span = ResourceLifeSpan::Manual);

  void Unload(LoadedHandle handle);

  template <typename Func>
  decltype(auto) WithLoaded(LoadedHandle handle, Func&& fn);

  template <typename Func>
  decltype(auto) WithLoaded(LoadedHandle handle, Func&& fn) const;

  template <typename Func>
  inline bool WithTemporaryLoad(Id id, Func&& fn);

  LoadedHandle RegisterDefaultResource(T* resource);

  virtual ResourceFile Pack(const T& resource,
                            CompressionMode compression_mode) = 0;
  virtual void Unpack(const ResourceFile& file, ResourceLifeSpan life_span,
                      T* resource) = 0;

 protected:
  inline static constexpr usize kDefaultResourceCapacity_{128};

  virtual void OnInitialize() {}
  virtual void OnDestroy() {}

  virtual void InitializeDefaults() {}
  virtual void DestroyDefaults() {}

  void ReleaseManagedResources();

  T* LoadInternal(Id id, ResourceLifeSpan life_span);
  void QueueForDeletion(T* resource);
  void DestroyDeleted();

  memory::Allocator* ResolveAllocator(memory::Allocator* default_allocator,
                                      ResourceLifeSpan life_span) const;

#ifdef COMET_PROFILING
  void SetupProfiling(const schar* method_name,
                      RawResourceId resource_id) const;
#define COMET_RESOURCE_HANDLER_SETUP_PROFILING(method_name, resource_id) \
  SetupProfiling(method_name, resource_id)
#else
#define COMET_RESOURCE_HANDLER_SETUP_PROFILING(method_name, resource_id)
#endif  // COMET_PROFILING

  void OnEvent(const event::Event& event);

  void RegisterEvents();
  void UnregisterEvents();

  event::EventListenerId scene_unloaded_listener_id_{};
  event::EventListenerId new_frame_listener_id_{};

  CTStringView root_path_{};

  memory::FiberFreeListAllocator resource_allocator_{};
  internal::LifeSpanAllocators life_span_allocators_{};

  internal::DefaultResources<T> defaults_{};
  internal::LoadingTracker<T> tracker_{};
  ResourceSlots<Tag, T> slots_{};
  HashSet<T*> deleted_resources_{};

  memory::Allocator* byte_allocator_{nullptr};

 private:
  bool is_initialized_{false};
};

template <typename Tag, typename T>
inline ResourceHandler<Tag, T>::ResourceHandler(
    const ResourceHandlerDescr& descr)
    : root_path_{descr.root_path},

      resource_allocator_{sizeof(T),
                          descr.initial_capacity != kInvalidSize
                              ? descr.initial_capacity
                              : kDefaultResourceCapacity_,
                          descr.memory_tag},
      life_span_allocators_{descr.life_span_allocators},

      defaults_{descr.ptr_allocator},
      tracker_{descr.ptr_allocator},
      slots_{descr.ptr_allocator, descr.initial_capacity != kInvalidSize
                                      ? descr.initial_capacity
                                      : kDefaultResourceCapacity_},
      deleted_resources_{descr.ptr_allocator, 64},

      byte_allocator_{descr.byte_allocator} {
  COMET_ASSERT(descr.ptr_allocator != nullptr,
               "ResourceHandler::ResourceHandler", "pointer allocator is null");
  COMET_ASSERT(descr.byte_allocator != nullptr,
               "ResourceHandler::ResourceHandler", "byte allocator is null");
  COMET_ASSERT(!descr.root_path.IsEmpty(), "ResourceHandler::ResourceHandler",
               "root path is empty");
}

template <typename Tag, typename T>
inline ResourceHandler<Tag, T>::~ResourceHandler() {
  COMET_ASSERT(!is_initialized_, "ResourceHandler::~ResourceHandler",
               "resource handler is still initialized");
}

template <typename Tag, typename T>
inline void ResourceHandler<Tag, T>::Initialize() {
  COMET_ASSERT(!is_initialized_, "ResourceHandler::Initialize",
               "resource handler is already initialized");

  defaults_.Initialize();
  tracker_.Initialize();
  slots_.Initialize();

  RegisterEvents();

  OnInitialize();
  InitializeDefaults();
  is_initialized_ = true;
}

template <typename Tag, typename T>
inline void ResourceHandler<Tag, T>::Destroy() {
  COMET_ASSERT(is_initialized_, "ResourceHandler::Destroy",
               "resource handler is not initialized");

  UnregisterEvents();
  ReleaseManagedResources();
  DestroyDeleted();
  DestroyDefaults();
  OnDestroy();

  defaults_.Destroy();
  tracker_.Destroy();
  slots_.Destroy();
  deleted_resources_.Destroy();

  is_initialized_ = false;
}

template <typename Tag, typename T>
inline T* ResourceHandler<Tag, T>::Get(LoadedHandle handle) {
  COMET_ASSERT(handle, "ResourceHandler::Get",
               "loaded resource handle is invalid");
  return slots_.Get(handle);
}

template <typename Tag, typename T>
inline const T* ResourceHandler<Tag, T>::Get(LoadedHandle handle) const {
  COMET_ASSERT(handle, "ResourceHandler::Get",
               "loaded resource handle is invalid");
  return slots_.Get(handle);
}

template <typename Tag, typename T>
inline typename ResourceHandler<Tag, T>::LoadedHandle
ResourceHandler<Tag, T>::Load(CTStringView path, ResourceLifeSpan life_span) {
  return Load(Id{GenerateResourceIdFromPath<T>(path)}, life_span);
}

template <typename Tag, typename T>
inline typename ResourceHandler<Tag, T>::LoadedHandle
ResourceHandler<Tag, T>::Load(Id id, ResourceLifeSpan life_span) {
  COMET_RESOURCE_HANDLER_SETUP_PROFILING("Load", id.GetValue());

  if (id.IsInvalid()) {
    return LoadedHandle::Invalid();
  }

  if (defaults_.TryGet(id.GetValue()) != nullptr) {
    // Default resources are pre-registered as immortal slot residents.
    const auto handle{slots_.TryGetHandle(id, ResourceLifeSpan::Immortal)};
    COMET_ASSERT(handle, "ResourceHandler::Load",
                 "default resource is not registered in slots", "resource_id",
                 id);

    return handle;
  }

  if (const auto handle{slots_.TryRetain(id, life_span)}; handle) {
    return handle;
  }

  bool is_already_loading;
  auto* loading_state{
      tracker_.RequestLoad(id.GetValue(), life_span, is_already_loading)};

  if (is_already_loading) {
    auto* resource{tracker_.Wait(loading_state)};

    if (resource == nullptr) {
      return LoadedHandle::Invalid();
    }

    const auto handle{slots_.TryRetain(Id{resource->id}, life_span)};

    if (!handle) {
      return LoadedHandle::Invalid();
    }

    return handle;
  }

  const auto handle{slots_.Acquire(
      id, life_span,
      [this](Id resource_id, ResourceLifeSpan resource_life_span) {
        return LoadInternal(resource_id, resource_life_span);
      },
      [this](T* duplicate_resource) { QueueForDeletion(duplicate_resource); })};

  auto* resource{slots_.Get(handle)};
  COMET_ASSERT(!handle || resource != nullptr, "ResourceHandler::Load",
               "loaded handle resolved to null resource", "resource_id", id,
               "life_span", GetResourceLifeSpanLabel(life_span),
               "life_span_value", ToUnderlying(life_span));

  tracker_.Finish(loading_state, resource);
  tracker_.Release(loading_state);

  return handle;
}

template <typename Tag, typename T>
inline void ResourceHandler<Tag, T>::Unload(LoadedHandle handle) {
  COMET_ASSERT(handle, "ResourceHandler::Unload",
               "loaded resource handle is invalid");

  auto* resource{slots_.Get(handle)};
  COMET_ASSERT(resource != nullptr, "ResourceHandler::Unload",
               "loaded resource is null");

  if (defaults_.IsDefault(resource->id)) {
    return;
  }

  slots_.Release(handle, [this](T* released_resource) {
    QueueForDeletion(released_resource);
  });
}

template <typename Tag, typename T>
inline typename ResourceHandler<Tag, T>::LoadedHandle
ResourceHandler<Tag, T>::RegisterDefaultResource(T* resource) {
  COMET_ASSERT(resource != nullptr, "ResourceHandler::RegisterDefaultResource",
               "default resource is null");
  COMET_ASSERT(resource->id != kInvalidRawResourceId,
               "ResourceHandler::RegisterDefaultResource",
               "default resource id is invalid");

  defaults_.Set(resource);

  const auto handle{slots_.RegisterImmortal(Id{resource->id}, resource)};
  COMET_ASSERT(handle, "ResourceHandler::RegisterDefaultResource",
               "default resource registration failed", "resource_id",
               resource->id);

  return handle;
}

template <typename Tag, typename T>
template <typename Func>
inline decltype(auto) ResourceHandler<Tag, T>::WithLoaded(LoadedHandle handle,
                                                          Func&& fn) {
  COMET_ASSERT(handle, "ResourceHandler::WithLoaded",
               "loaded resource handle is invalid");

  auto* resource{Get(handle)};
  COMET_ASSERT(resource != nullptr, "ResourceHandler::WithLoaded",
               "loaded resource is null");

  return std::invoke(std::forward<Func>(fn), resource);
}

template <typename Tag, typename T>
template <typename Func>
inline decltype(auto) ResourceHandler<Tag, T>::WithLoaded(LoadedHandle handle,
                                                          Func&& fn) const {
  COMET_ASSERT(handle, "ResourceHandler::WithLoaded",
               "loaded resource handle is invalid");

  const auto* resource{Get(handle)};
  COMET_ASSERT(resource != nullptr, "ResourceHandler::WithLoaded",
               "loaded resource is null");

  return std::invoke(std::forward<Func>(fn), resource);
}

template <typename Tag, typename T>
template <typename Func>
inline bool ResourceHandler<Tag, T>::WithTemporaryLoad(Id id, Func&& fn) {
  const auto handle{Load(id, ResourceLifeSpan::Manual)};

  if (!handle) {
    return false;
  }

  internal::LoadedResourceScope<ResourceHandler<Tag, T>> scope{*this, handle};

  auto* resource{Get(handle)};
  COMET_ASSERT(resource != nullptr, "ResourceHandler::WithTemporaryLoad",
               "loaded resource is null", "resource_id", id.GetValue());

  std::invoke(std::forward<Func>(fn), resource);
  return true;
}

template <typename Tag, typename T>
inline void ResourceHandler<Tag, T>::ReleaseManagedResources() {
  slots_.ReleaseAll(ResourceLifeSpan::Manual, [this](T* released_resource) {
    QueueForDeletion(released_resource);
  });

  slots_.ReleaseAll(ResourceLifeSpan::Scene, [this](T* released_resource) {
    QueueForDeletion(released_resource);
  });

  slots_.ReleaseAll(ResourceLifeSpan::Global, [this](T* released_resource) {
    QueueForDeletion(released_resource);
  });
}

template <typename Tag, typename T>
inline T* ResourceHandler<Tag, T>::LoadInternal(Id id,
                                                ResourceLifeSpan life_span) {
  COMET_RESOURCE_HANDLER_SETUP_PROFILING("LoadInternal", id.GetValue());

  ResourceFile file{};
  file.descr = Array<u8>{byte_allocator_};
  file.data = Array<u8>{byte_allocator_};

  const auto& resource_abs_path{
      internal::GenerateTlsResourceAbsPath(root_path_, id.GetValue())};
  COMET_ASSERT(!resource_abs_path.IsEmpty(), "ResourceHandler::LoadInternal",
               "resource path is empty", "resource_id", id);

  T* resource{nullptr};

  if (LoadResourceFile(resource_abs_path, file)) {
    resource = resource_allocator_.AllocateOneAndPopulate<T>();
    COMET_ASSERT(resource != nullptr, "ResourceHandler::LoadInternal",
                 "resource allocation failed", "resource_id", id);

    Unpack(file, life_span, resource);
    return resource;
  }

  COMET_LOG_ERROR(LoggerType::Resource, "ResourceHandler::LoadInternal",
                  "resource file load failed", "resource_id", id,
                  "resource_label", COMET_STRING_ID_LABEL(id.GetValue()));

  return nullptr;
}

template <typename Tag, typename T>
inline void ResourceHandler<Tag, T>::QueueForDeletion(T* resource) {
  if (resource == nullptr || defaults_.IsDefault(resource->id)) {
    return;
  }

  deleted_resources_.Add(resource);
}

template <typename Tag, typename T>
inline void ResourceHandler<Tag, T>::DestroyDeleted() {
  for (auto* resource : deleted_resources_) {
    COMET_ASSERT(resource != nullptr, "ResourceHandler::DestroyDeleted",
                 "deleted resource is null");

    resource->~T();
    resource_allocator_.Deallocate(resource);
  }

  deleted_resources_.Clear();
}

template <typename Tag, typename T>
inline memory::Allocator* ResourceHandler<Tag, T>::ResolveAllocator(
    memory::Allocator* default_allocator, ResourceLifeSpan life_span) const {
  switch (life_span) {
    case ResourceLifeSpan::Scene:
      return life_span_allocators_.scene;

    case ResourceLifeSpan::Global:
      return life_span_allocators_.global;

    case ResourceLifeSpan::Manual:
      return default_allocator;

    case ResourceLifeSpan::Immortal:
      return life_span_allocators_.immortal;

    default:
      COMET_ASSERT(false, "ResourceHandler::ResolveAllocator",
                   "resource life span is unsupported", "life_span",
                   GetResourceLifeSpanLabel(life_span), "life_span_value",
                   ToUnderlying(life_span));
      return nullptr;
  }
}

template <typename Tag, typename T>
inline void ResourceHandler<Tag, T>::OnEvent(const event::Event& event) {
  const auto& event_type{event.GetType()};

  if (event_type == scene::SceneUnloadedEvent::kStaticType_) {
    slots_.ReleaseAll(ResourceLifeSpan::Scene, [this](T* released_resource) {
      QueueForDeletion(released_resource);
    });
  } else if (event_type == frame::NewFrameEvent::kStaticType_) {
    DestroyDeleted();
  }
}

template <typename Tag, typename T>
inline void ResourceHandler<Tag, T>::RegisterEvents() {
  auto& event_manager{event::EventManager::Get()};
  const auto event_function{
      [this](const event::Event& event) { OnEvent(event); }};

  scene_unloaded_listener_id_ = event_manager.Register(
      event_function, scene::SceneUnloadedEvent::kStaticType_);
  COMET_ASSERT(scene_unloaded_listener_id_ != event::kInvalidEventListenerId,
               "ResourceHandler::Initialize",
               "scene unloaded listener registration failed");

  new_frame_listener_id_ = event_manager.Register(
      event_function, frame::NewFrameEvent::kStaticType_);
  COMET_ASSERT(new_frame_listener_id_ != event::kInvalidEventListenerId,
               "ResourceHandler::Initialize",
               "new frame listener registration failed");
}

template <typename Tag, typename T>
inline void ResourceHandler<Tag, T>::UnregisterEvents() {
  auto& event_manager{event::EventManager::Get()};

  if (scene_unloaded_listener_id_ != event::kInvalidEventListenerId) {
    event_manager.Unregister(scene_unloaded_listener_id_);
    scene_unloaded_listener_id_ = event::kInvalidEventListenerId;
  }

  if (new_frame_listener_id_ != event::kInvalidEventListenerId) {
    event_manager.Unregister(new_frame_listener_id_);
    new_frame_listener_id_ = event::kInvalidEventListenerId;
  }
}

#ifdef COMET_PROFILING
template <typename Tag, typename T>
inline void ResourceHandler<Tag, T>::SetupProfiling(
    const schar* method_name, RawResourceId resource_id) const {
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

#endif  // COMET_COMET_RESOURCE_HANDLER_RESOURCE_HANDLER_H_