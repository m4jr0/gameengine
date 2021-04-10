// Copyright 2021 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RESOURCE_MODEL_RESOURCE_H_
#define COMET_COMET_RESOURCE_MODEL_RESOURCE_H_

#include "comet/game_object/model/model.h"
#include "comet_precompile.h"
#include "resource.h"

namespace comet {
namespace resource {
class ModelResource : public Resource {
 public:
  ModelResource(const std::string&);
  ModelResource(const ModelResource&);
  ModelResource(ModelResource&&) noexcept;
  ModelResource& operator=(const ModelResource&);
  ModelResource& operator=(ModelResource&&) noexcept;
  virtual ~ModelResource() = default;

  virtual std::shared_ptr<Component> Clone() const override;
  virtual void Destroy() override;
  virtual bool Import() override;
  virtual bool Export() override;
  virtual bool Dump() override;
  virtual bool Load() override;

  virtual std::shared_ptr<game_object::Model> GetModel();

 protected:
  std::shared_ptr<game_object::Model> model_ = nullptr;
};
}  // namespace resource
}  // namespace comet

#endif  // COMET_COMET_RESOURCE_MODEL_RESOURCE_H_
