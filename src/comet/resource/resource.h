// Copyright 2021 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RESOURCE_RESOURCE_H_
#define COMET_COMET_RESOURCE_RESOURCE_H_

#include "boost/uuid/uuid.hpp"
#include "boost/uuid/uuid_generators.hpp"
#include "boost/uuid/uuid_io.hpp"
#include "comet/game_object/component.h"
#include "comet/resource/resource_manager.h"
#include "comet_precompile.h"
#include "nlohmann/json.hpp"

namespace comet {
namespace resource {
class Resource : public game_object::Component {
 public:
  friend ResourceManager;

  Resource(const std::string&);
  Resource(const Resource&);
  Resource(Resource&&) noexcept;
  Resource& operator=(const Resource&);
  Resource& operator=(Resource&&) noexcept;
  virtual ~Resource() = default;

  virtual void Destroy() = 0;

  const boost::uuids::uuid& GetId() const noexcept;

 protected:
  virtual void SetMetaFile();
  virtual void Initialize();
  virtual void Update();
  virtual bool Delete();
  virtual bool Import() = 0;
  virtual bool Export() = 0;
  virtual bool Dump() = 0;
  virtual bool Load() = 0;

  virtual const nlohmann::json& GetMetaData() const;

  double creation_time_;
  double modification_time_;
  std::string file_system_path_;
  std::string file_system_name_;
  boost::uuids::uuid id_ = boost::uuids::random_generator()();
  nlohmann::json meta_data_;
};
}  // namespace resource
}  // namespace comet

#endif  // COMET_COMET_RESOURCE_RESOURCE_H_
