// Copyright 2021 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "resource.hpp"

#include <utils/date.hpp>
#include <utils/file_system.hpp>
#include <utils/logger.hpp>

#ifdef _WIN32
// Allow debugging memory leaks on Windows.
#include <debug_windows.hpp>
#endif  // _WIN32

namespace koma {
const boost::uuids::uuid Resource::kId() const noexcept {
  return this->kId_;
}

Resource::Resource(const std::string &path) {
  this->file_system_path_ = path;
  this->file_system_name_ = filesystem::GetName(path);
}

void Resource::SetMetaFile() {
  nlohmann::json resource_meta_data;
  double now = date::GetNow();

  if (filesystem::IsExist(this->file_system_path_)) {
    std::string raw_meta_data;

    resource_meta_data = filesystem::ReadFile(
      this->file_system_path_,
      &raw_meta_data
    );

    resource_meta_data = nlohmann::json::parse(raw_meta_data);
    resource_meta_data["id"] = this->kId_;
    resource_meta_data["modification_time"] = now;
  } else {
    resource_meta_data["id"] = this->kId_;
    resource_meta_data["creation_time"] = now;
    resource_meta_data["modification_time"] = now;
  }

  resource_meta_data["checksum"] =
    filesystem::GetChecksum(this->file_system_path_);

  resource_meta_data["data"] = this->GetMetaData();

  if (!filesystem::WriteToFile(this->file_system_path_,
                               resource_meta_data.dump(2))) {
    Logger::Get(LOGGER_KOMA_CORE_RESOURCE_RESOURCE)->Error(
      "Could not write the resource meta file at path ",
      this->file_system_path_
    );
  }
}

void Resource::Initialize() {
  this->creation_time_ = date::GetNow();
  this->modification_time_ = this->creation_time_;

  this->SetMetaFile();
}

void Resource::Update() {
  this->modification_time_ = date::GetNow();

  this->SetMetaFile();
}

bool Resource::Delete() {
  return true;
}
}  // namespace koma
