// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RESOURCE_RESOURCE_MANAGER_H_
#define COMET_COMET_RESOURCE_RESOURCE_MANAGER_H_

#include "comet/core/essentials.h"
#include "comet/core/logger.h"
#include "comet/core/manager.h"
#include "comet/core/memory/allocator/stack_allocator.h"
#include "comet/core/memory/memory.h"
#include "comet/core/type/tstring.h"
#include "comet/resource/resource.h"

namespace comet {
namespace resource {
class ResourceManager : public Manager {
 public:
  static ResourceManager& Get();

  ResourceManager() = default;
  ResourceManager(const ResourceManager&) = delete;
  ResourceManager(ResourceManager&&) = delete;
  ResourceManager& operator=(const ResourceManager&) = delete;
  ResourceManager& operator=(ResourceManager&&) = delete;
  virtual ~ResourceManager() = default;

  void Initialize() override;
  void Shutdown() override;
  void InitializeResourcesDirectory();

  const TString& GetRootResourcePath();

  template <typename ResourceHandlerType>
  void AddHandler(ResourceTypeId resource_type_id) {
    if (handlers_.IsContained(resource_type_id)) {
      COMET_LOG_RESOURCE_ERROR(
          "Adding handler to an already known resource type ID: ",
          resource_type_id, ". Aborting.");
      return;
    }

    auto& pair{handlers_.Emplace(
        resource_type_id,
        handler_allocator_.AllocateOneAndPopulate<ResourceHandlerType>(
            &loading_resources_allocator_, &loading_resource_allocator_))};

    pair.value->Initialize();
  }

  void RemoveHandler(ResourceTypeId resource_type_id);

  template <typename ResourceType>
  const ResourceType* Load(
      ResourceId resource_id,
      [[maybe_unused]] ResourceLifeSpan life_span = ResourceLifeSpan::Manual) {
    COMET_ASSERT(handlers_.IsContained(ResourceType::kResourceTypeId),
                 "Unknown resource type ID: ", ResourceType::kResourceTypeId,
                 ". Aborting.");
    auto* handler{handlers_.Get(ResourceType::kResourceTypeId)};

#ifdef COMET_DEBUG

    const auto* resource{static_cast<const ResourceType*>(handler->Load(
        resource_file_allocator_, root_resource_path_, resource_id))};

    if (resource != nullptr) {
      COMET_ASSERT(resource->type_id == ResourceType::kResourceTypeId,
                   "Invalid resource type provided. ID of expected type is ",
                   COMET_STRING_ID_LABEL(resource->type_id),
                   ", ID of type provided is ",
                   COMET_STRING_ID_LABEL(ResourceType::kResourceTypeId));
    }

    return resource;
#else
    return static_cast<const ResourceType*>(
        handler->Load(root_resource_path_, resource_id));
#endif  // COMET_DEBUG
  }

  template <typename ResourceType>
  const ResourceType* Load(
      CTStringView asset_path,
      ResourceLifeSpan life_span = ResourceLifeSpan::Manual) {
    COMET_ASSERT(!asset_path.IsEmpty(), "Asset path provided is empty!");
    return Load<ResourceType>(GenerateResourceIdFromPath(asset_path),
                              life_span);
  }

  template <typename ResourceType>
  void Unload(ResourceId resource_id) {
    COMET_ASSERT(handlers_.IsContained(ResourceType::kResourceTypeId),
                 "Unknown resource type ID: ", ResourceType::kResourceTypeId,
                 ". Aborting.");
    auto* handler{handlers_[ResourceType::kResourceTypeId].get()};
    handler->Unload(resource_file_allocator_, root_resource_path_, resource_id);
  }

  template <typename ResourceType>
  void Unload(CTStringView asset_path) {
    Unload<ResourceType>(GenerateResourceIdFromPath(asset_path));
  }

  template <typename ResourceType>
  ResourceFile GetResourceFile(const ResourceType& resource,
                               CompressionMode compression_mode) {
    COMET_ASSERT(handlers_.IsContained(resource.kResourceTypeId),
                 "Unknown resource ID: ", resource.kResourceTypeId);

    auto* handler{handlers_.Get(resource.kResourceTypeId)};
    return handler->GetResourceFile(resource_file_allocator_, resource,
                                    compression_mode);
  }

 private:
  using ResourceHandlers = Map<ResourceId, ResourceHandler*>;

  TString root_resource_path_{};
  // Good enough for something that will never be updated...
  memory::PlatformAllocator handler_allocator_{
      memory::kEngineMemoryTagResource};
  memory::FiberFreeListAllocator loading_resources_allocator_{
      sizeof(internal::LoadingResourceState*), 1024,
      memory::kEngineMemoryTagResource};
  memory::FiberFreeListAllocator loading_resource_allocator_{
      sizeof(internal::LoadingResourceState), 1024,
      memory::kEngineMemoryTagResource};
  memory::FiberFreeListAllocator resource_file_allocator_{
      1024, 2048, memory::kEngineMemoryTagResource};
  ResourceHandlers handlers_{};
};
}  // namespace resource
}  // namespace comet

#endif  // COMET_COMET_RESOURCE_RESOURCE_MANAGER_H_
