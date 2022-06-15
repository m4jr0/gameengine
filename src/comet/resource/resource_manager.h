// Copyright 2022 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RESOURCE_RESOURCE_MANAGER_H_
#define COMET_COMET_RESOURCE_RESOURCE_MANAGER_H_

#include "comet_precompile.h"

#include "comet/resource/resource.h"
#include "comet/utils/file_system.h"

namespace comet {
namespace resource {
class ResourceCache {
 public:
  ResourceCache() = default;
  ResourceCache(const ResourceCache&) = delete;
  ResourceCache(ResourceCache&&) = delete;
  ResourceCache& operator=(const ResourceCache&) = delete;
  ResourceCache& operator=(ResourceCache&&) = delete;
  ~ResourceCache() = default;

  void Set(std::unique_ptr<Resource> resource);
  const Resource* Get(ResourceId resource_id);

 private:
  std::unordered_map<ResourceId, std::unique_ptr<Resource>> cache_;
};

class ResourceManager {
 public:
  ResourceManager() = default;
  ResourceManager(const ResourceManager&) = delete;
  ResourceManager(ResourceManager&&) = delete;
  ResourceManager& operator=(const ResourceManager&) = delete;
  ResourceManager& operator=(ResourceManager&&) = delete;
  ~ResourceManager() = default;

  void Initialize();
  void Destroy();
  void InitializeResourcesDirectory();

  const std::string& GetRootResourcePath();

  template <typename ResourceHandlerType>
  void AddHandler(ResourceId resource_id) {
    if (handlers_.find(resource_id) != handlers_.cend()) {
      COMET_LOG_RESOURCE_ERROR(
          "Adding handler to an already known resource ID: ", resource_id,
          ". Aborting.");
      return;
    }

    handlers_[resource_id] = std::make_unique<ResourceHandlerType>();
  }

  template <typename ResourceType>
  const ResourceType* LoadFromResourceId(
      ResourceId resource_id,
      ResourceLifeSpan life_span = ResourceLifeSpan::Global) {
    const auto* resource{cache_.Get(resource_id)};

    if (resource != nullptr) {
      COMET_ASSERT(resource->type_id == ResourceType::kResourceTypeId,
                   "Invalid resource type provided. ID of expected type is ",
                   resource->type_id, ", ID of type provided is ",
                   ResourceType::kResourceTypeId);

      return static_cast<const ResourceType*>(resource);
    }

    COMET_ASSERT(
        handlers_.find(ResourceType::kResourceTypeId) != handlers_.cend(),
        "Unknown resource type ID: ", ResourceType::kResourceTypeId,
        ". Aborting.");

    const auto* handler{handlers_.at(ResourceType::kResourceTypeId).get()};
    cache_.Set(handler->Load(root_resource_path_, std::to_string(resource_id)));
    return static_cast<const ResourceType*>(cache_.Get(resource_id));
  }

  template <typename ResourceType, typename AssetPath>
  const ResourceType* Load(
      AssetPath&& asset_path,
      ResourceLifeSpan life_span = ResourceLifeSpan::Global) {
    return LoadFromResourceId<ResourceType>(
        GenerateResourceIdFromPath(asset_path), life_span);
  }

  template <typename ResourceType>
  ResourceFile GetResourceFile(const ResourceType& resource,
                               CompressionMode compression_mode) {
    COMET_ASSERT(handlers_.find(resource.kResourceTypeId) != handlers_.cend(),
                 "Unknown resource ID: ", resource.kResourceTypeId);

    auto* handler{handlers_.at(resource.kResourceTypeId).get()};
    return handler->GetResourceFile(resource, compression_mode);
  }

 private:
  std::string root_resource_path_;
  std::unordered_map<ResourceId, std::unique_ptr<ResourceHandler>> handlers_;
  ResourceCache cache_;
};
}  // namespace resource
}  // namespace comet

#endif  // COMET_COMET_RESOURCE_RESOURCE_MANAGER_H_
