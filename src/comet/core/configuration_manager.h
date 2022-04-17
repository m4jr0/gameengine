// Copyright 2022 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_CONFIGURATION_MANAGER_H_
#define COMET_COMET_CORE_CONFIGURATION_MANAGER_H_

#include <any>

#include "comet/core/manager.h"
#include "comet_precompile.h"

namespace comet {
namespace core {
class ConfigurationManager : public core::Manager {
 public:
  ConfigurationManager() = default;
  ConfigurationManager(const ConfigurationManager&) = delete;
  ConfigurationManager(ConfigurationManager&&) = delete;
  ConfigurationManager& operator=(const ConfigurationManager&) = delete;
  ConfigurationManager& operator=(ConfigurationManager&&) = delete;
  virtual ~ConfigurationManager() = default;

  virtual void Initialize() override;

  template <typename T>
  static void Set(const std::string entry, T value) {
    values_[entry] = value;
  }

  template <typename T>
  static T Get(const std::string entry) {
    return std::any_cast<T>(values_[entry]);
  }

 private:
  static std::unordered_map<std::string, std::any> values_;
};
}  // namespace core
}  // namespace comet

#endif  // COMET_COMET_CORE_CONFIGURATION_MANAGER_H_