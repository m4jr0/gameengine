// Copyright 2022 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "resource.h"

#include "comet/utils/date.h"
#include "comet/utils/file_system.h"

#ifdef _WIN32
#include "debug_windows.h"
#endif  // _WIN32

namespace comet {
namespace resource {
Resource::Resource(const std::string& path) {
  file_system_path_ = path;
  file_system_name_ = utils::filesystem::GetName(path);
}

Resource::Resource(const Resource& other)
    : comet::game_object::Component(other),
      creation_time_(other.creation_time_),
      modification_time_(other.modification_time_),
      file_system_path_(other.file_system_path_),
      file_system_name_(other.file_system_name_),
      id_(boost::uuids::random_generator()()),
      meta_data_(other.meta_data_) {}

Resource::Resource(Resource&& other) noexcept
    : comet::game_object::Component(std::move(other)),
      creation_time_(std::move(other.creation_time_)),
      modification_time_(std::move(other.modification_time_)),
      file_system_path_(std::move(other.file_system_path_)),
      file_system_name_(std::move(other.file_system_name_)),
      id_(boost::uuids::random_generator()()),
      meta_data_(std::move(other.meta_data_)) {}

Resource& Resource::operator=(const Resource& other) {
  if (this == &other) {
    return *this;
  }

  Component::operator=(other);
  creation_time_ = other.creation_time_;
  modification_time_ = other.modification_time_;
  file_system_path_ = other.file_system_path_;
  file_system_name_ = other.file_system_name_;
  id_ = boost::uuids::random_generator()();
  meta_data_ = other.meta_data_;
  return *this;
}

Resource& Resource::operator=(Resource&& other) noexcept {
  if (this == &other) {
    return *this;
  }

  Component::operator=(other);
  creation_time_ = std::move(other.creation_time_);
  modification_time_ = std::move(other.modification_time_);
  file_system_path_ = std::move(other.file_system_path_);
  file_system_name_ = std::move(other.file_system_name_);
  id_ = boost::uuids::random_generator()();
  meta_data_ = std::move(other.meta_data_);
  return *this;
}

void Resource::SetMetaFile() {
  meta_data_.empty();
  const auto now = utils::date::GetNow();

  if (utils::filesystem::IsExist(file_system_path_)) {
    std::string raw_meta_data;

    meta_data_ = utils::filesystem::ReadFile(file_system_path_, &raw_meta_data);

    meta_data_ = nlohmann::json::parse(raw_meta_data);
    meta_data_["id"] = id_;
    meta_data_["modification_time"] = now;
  } else {
    meta_data_["id"] = id_;
    meta_data_["creation_time"] = now;
    meta_data_["modification_time"] = now;
  }

  meta_data_["checksum"] = utils::filesystem::GetChecksum(file_system_path_);
  meta_data_["data"] = GetMetaData();

  if (!utils::filesystem::WriteToFile(file_system_path_, meta_data_.dump(2))) {
    core::Logger::Get(core::LoggerType::Resource)
        .Error("Could not write the resource meta file at path ",
               file_system_path_);
  }
}

void Resource::Initialize() {
  creation_time_ = utils::date::GetNow();
  modification_time_ = creation_time_;
  SetMetaFile();
}

void Resource::Update() {
  modification_time_ = utils::date::GetNow();
  SetMetaFile();
}

bool Resource::Delete() { return true; }

const boost::uuids::uuid& Resource::GetId() const noexcept { return id_; }

const nlohmann::json& Resource::GetMetaData() const { return meta_data_; }
}  // namespace resource
}  // namespace comet
