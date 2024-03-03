// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RESOURCE_RESOURCE_MANAGER_H_
#define COMET_COMET_RESOURCE_RESOURCE_MANAGER_H_

#include "comet_precompile.h"

#include "comet/core/manager.h"
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
  const ResourceType* Load(
      ResourceId resource_id,
      ResourceLifeSpan life_span = ResourceLifeSpan::Manual) {
    COMET_ASSERT(
        handlers_.find(ResourceType::kResourceTypeId) != handlers_.cend(),
        "Unknown resource type ID: ", ResourceType::kResourceTypeId,
        ". Aborting.");
    auto* handler{handlers_.at(ResourceType::kResourceTypeId).get()};

#ifdef COMET_DEBUG
    const auto* resource{static_cast<const ResourceType*>(
        handler->Load(root_resource_path_, resource_id))};

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
    COMET_ASSERT(
        handlers_.find(ResourceType::kResourceTypeId) != handlers_.cend(),
        "Unknown resource type ID: ", ResourceType::kResourceTypeId,
        ". Aborting.");
    auto* handler{handlers_.at(ResourceType::kResourceTypeId).get()};
    handler->Unload(root_resource_path_, resource_id);
  }

  template <typename ResourceType>
  void Unload(CTStringView asset_path) {
    Unload<ResourceType>(GenerateResourceIdFromPath(asset_path));
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
  TString root_resource_path_{};
  std::unordered_map<ResourceId, std::unique_ptr<ResourceHandler>> handlers_{};
};
}  // namespace resource
}  // namespace comet

#endif  // COMET_COMET_RESOURCE_RESOURCE_MANAGER_H_
