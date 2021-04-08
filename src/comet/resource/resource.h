// Copyright 2021 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RESOURCE_RESOURCE_H_
#define COMET_COMET_RESOURCE_RESOURCE_H_

constexpr auto kLoggerCometCoreResourceResource = "COMET_COMET_core_resource";

#include "boost/uuid/uuid.hpp"
#include "boost/uuid/uuid_generators.hpp"
#include "boost/uuid/uuid_io.hpp"
#include "comet/game_object/component.h"
#include "comet/resource/resource_manager.h"
#include "comet_precompile.h"
#include "nlohmann/json.hpp"

namespace comet {
class Resource : public Component {
 public:
  friend ResourceManager;

  Resource(const std::string &);

  virtual void Destroy() = 0;

  const boost::uuids::uuid kId() const noexcept;

 protected:
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

  double creation_time_;
  double modification_time_;
  std::string file_system_path_;
  std::string file_system_name_;
  const boost::uuids::uuid kId_ = boost::uuids::random_generator()();
};
}  // namespace comet

#endif  // COMET_COMET_RESOURCE_RESOURCE_H_
