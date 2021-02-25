// Copyright 2021 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef KOMA_CORE_RESOURCE_MODEL_RESOURCE_HPP_
#define KOMA_CORE_RESOURCE_MODEL_RESOURCE_HPP_

#define LOGGER_KOMA_CORE_RESOURCE_MODEL_RESOURCE "koma_core_resource"

#include "resource.hpp"

#include <memory>

#include <core/game_object/model/model.hpp>

namespace koma {
class ModelResource : public Resource {
 public:
  ModelResource(const std::string &path) : Resource(path){};

  virtual void Destroy() override;

  virtual bool Import() override;
  virtual bool Export() override;
  virtual bool Dump() override;
  virtual bool Load() override;

  virtual nlohmann::json GetMetaData() override;
  virtual std::shared_ptr<Model> GetModel() { return this->model_; };

 protected:
  std::shared_ptr<Model> model_ = nullptr;
};
}  // namespace koma

#endif  // KOMA_CORE_RESOURCE_MODEL_RESOURCE_HPP_
