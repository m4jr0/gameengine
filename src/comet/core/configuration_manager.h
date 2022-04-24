// Copyright 2022 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_CONFIGURATION_MANAGER_H_
#define COMET_COMET_CORE_CONFIGURATION_MANAGER_H_

#include "comet_precompile.h"

#include <any>

namespace comet {
namespace conf {
class ConfigurationManager {
 public:
  ConfigurationManager() = default;
  ConfigurationManager(const ConfigurationManager&) = delete;
  ConfigurationManager(ConfigurationManager&&) = delete;
  ConfigurationManager& operator=(const ConfigurationManager&) = delete;
  ConfigurationManager& operator=(ConfigurationManager&&) = delete;
  ~ConfigurationManager() = default;

  void Initialize();
  void Destroy();

  template <typename T, typename String>
  void Set(String&& entry, T&& value) {
    values_[std::forward<String>(entry)] = std::forward<T>(value);
  }

  template <typename T, typename String>
  T Get(String&& entry) const {
    return std::any_cast<T>(values_.at(std::forward<String>(entry)));
  }

 private:
  std::unordered_map<std::string, std::any> values_;
};
}  // namespace conf
}  // namespace comet

#endif  // COMET_COMET_CORE_CONFIGURATION_MANAGER_H_