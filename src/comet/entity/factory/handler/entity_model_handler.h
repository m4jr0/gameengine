// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_ENTITY_FACTORY_HANDLER_ENTITY_MODEL_HANDLER_H_
#define COMET_COMET_ENTITY_FACTORY_HANDLER_ENTITY_MODEL_HANDLER_H_

#include "comet/core/concurrency/job/job.h"
#include "comet/core/essentials.h"
#include "comet/core/type/map.h"
#include "comet/core/type/tstring.h"
#include "comet/entity/entity_id.h"
#include "comet/entity/factory/handler/entity_handler.h"
#include "comet/resource/model_resource.h"
#include "comet/resource/resource.h"

namespace comet {
namespace entity {
class ModelHandler : public Handler {
 public:
  ModelHandler() = default;
  ModelHandler(const ModelHandler&) = delete;
  ModelHandler(ModelHandler&&) = delete;
  ModelHandler& operator=(const ModelHandler&) = delete;
  ModelHandler& operator=(ModelHandler&&) = delete;
  virtual ~ModelHandler() = default;

  EntityId GenerateStatic(CTStringView model_path,
                          resource::ResourceLifeSpan life_span =
                              resource::ResourceLifeSpan::Manual) const;
  EntityId GenerateSkeletal(CTStringView model_path,
                            resource::ResourceLifeSpan life_span =
                                resource::ResourceLifeSpan::Manual) const;

  void DestroyStatic(EntityId entity_id) const;
  void DestroySkeletal(EntityId entity_id) const;

 private:
  static constexpr inline usize kMaxConcurrentJobs_{10};

  void DestroyStaticChildren(EntityId current_entity_id) const;
  void DestroySkeletalChildren(EntityId current_entity_id) const;

  static void OnStaticGeneration(job::JobParamsHandle params_handle);
  static void OnSkeletalGeneration(job::JobParamsHandle params_handle);
};

namespace internal {
using ParentEntityIds = Map<resource::ResourceId, EntityId>;

struct StaticGenerationJobParams {
  resource::ResourceLifeSpan life_span{resource::ResourceLifeSpan::Manual};
  EntityId id{kInvalidEntityId};
  EntityId root_entity_id{kInvalidEntityId};
  EntityId parent_id{kInvalidEntityId};
  const resource::StaticMeshResource* mesh{nullptr};
};

struct SkeletalGenerationJobParams {
  resource::ResourceLifeSpan life_span{resource::ResourceLifeSpan::Manual};
  EntityId id{kInvalidEntityId};
  EntityId root_entity_id{kInvalidEntityId};
  EntityId parent_id{kInvalidEntityId};
  const resource::SkinnedMeshResource* mesh{nullptr};
};
}  // namespace internal
}  // namespace entity
}  // namespace comet

#endif  // COMET_COMET_ENTITY_FACTORY_HANDLER_ENTITY_MODEL_HANDLER_H_
