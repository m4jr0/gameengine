// Copyright 2021 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "resource.hpp"

#include "utils/date.hpp"
#include "utils/file_system.hpp"
#include "utils/logger.hpp"

#ifdef _WIN32
// Allow debugging memory leaks on Windows.
#include "debug_windows.hpp"
#endif  // _WIN32

namespace koma {
const boost::uuids::uuid Resource::kId() const noexcept { return kId_; }

Resource::Resource(const std::string &path) {
  file_system_path_ = path;
  file_system_name_ = filesystem::GetName(path);
}

void Resource::SetMetaFile() {
  nlohmann::json resource_meta_data{};
  const auto now = date::GetNow();

  if (filesystem::IsExist(file_system_path_)) {
    std::string raw_meta_data;

    resource_meta_data =
        filesystem::ReadFile(file_system_path_, &raw_meta_data);

    resource_meta_data = nlohmann::json::parse(raw_meta_data);
    resource_meta_data["id"] = kId_;
    resource_meta_data["modification_time"] = now;
  } else {
    resource_meta_data["id"] = kId_;
    resource_meta_data["creation_time"] = now;
    resource_meta_data["modification_time"] = now;
  }

  resource_meta_data["checksum"] = filesystem::GetChecksum(file_system_path_);
  resource_meta_data["data"] = GetMetaData();

  if (!filesystem::WriteToFile(file_system_path_, resource_meta_data.dump(2))) {
    Logger::Get(kLoggerKomaCoreResourceResource)
        ->Error("Could not write the resource meta file at path ",
                file_system_path_);
  }
}

void Resource::Initialize() {
  creation_time_ = date::GetNow();
  modification_time_ = creation_time_;
  SetMetaFile();
}

void Resource::Update() {
  modification_time_ = date::GetNow();
  SetMetaFile();
}

bool Resource::Delete() { return true; }
}  // namespace koma
