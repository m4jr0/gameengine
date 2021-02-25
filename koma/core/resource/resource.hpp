// Copyright 2021 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef KOMA_CORE_RESOURCE_RESOURCE_HPP_
#define KOMA_CORE_RESOURCE_RESOURCE_HPP_

#define LOGGER_KOMA_CORE_RESOURCE_RESOURCE "koma_core_resource"

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <memory>
#include <nlohmann/json.hpp>
#include <string>

#include "core/game_object/component.hpp"
#include "resource_manager.hpp"

namespace koma {
class Resource : public Component {
 public:
  const boost::uuids::uuid kId() const noexcept;
  Resource(const std::string &);

  virtual void Destroy() = 0;

  friend ResourceManager;

 protected:
  double creation_time_;
  double modification_time_;
  std::string file_system_path_;
  std::string file_system_name_;
  const boost::uuids::uuid kId_ = boost::uuids::random_generator()();

  Resource() = delete;
  virtual void SetMetaFile();
  virtual nlohmann::json GetMetaData() = 0;
  virtual void Initialize();
  virtual void Update();
  virtual bool Delete();
  virtual bool Import() = 0;
  virtual bool Export() = 0;
  virtual bool Dump() = 0;
  virtual bool Load() = 0;
};
}  // namespace koma

#endif  // KOMA_CORE_RESOURCE_RESOURCE_HPP_
