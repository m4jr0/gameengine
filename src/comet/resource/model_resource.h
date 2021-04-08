// Copyright 2021 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RESOURCE_MODEL_RESOURCE_H_
#define COMET_COMET_RESOURCE_MODEL_RESOURCE_H_

constexpr auto kLoggerCometCoreResourceModelResource = "comet_core_resource";

#include "comet/game_object/model/model.h"
#include "comet_precompile.h"
#include "resource.h"

namespace comet {
class ModelResource : public Resource {
 public:
  ModelResource(const std::string &path) : Resource(path){};

  virtual void Destroy() override;
  virtual bool Import() override;
  virtual bool Export() override;
  virtual bool Dump() override;
  virtual bool Load() override;
  virtual nlohmann::json GetMetaData() override;
  virtual std::shared_ptr<Model> GetModel() { return model_; };

 protected:
  std::shared_ptr<Model> model_ = nullptr;
};
}  // namespace comet

#endif  // COMET_COMET_RESOURCE_MODEL_RESOURCE_H_
