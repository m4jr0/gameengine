// Copyright 2022 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_CONFIGURATION_MANAGER_H_
#define COMET_COMET_CORE_CONFIGURATION_MANAGER_H_

#include "comet_precompile.h"

namespace comet {
namespace conf {
using ConfKey = stringid::StringId;

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

  template <typename String>
  static ConfKey GenerateConfKey(String&& str_key) {
    return COMET_STRING_ID(str_key);
  }

  template <typename T>
  void Set(ConfKey key, T&& value) {
    static_assert(
        std::is_fundamental<T>::value || std::is_same<T, std::string>::value,
        "T must be fundamental or a std::string!");
    values_[key] = std::forward<T>(value);
  }

  template <typename T, typename String>
  void Set(String&& str_key, T&& value) {
    Set(GenerateConfKey(str_key), std::forward<T>(value));
  }

  template <typename T>
  T Get(ConfKey key) const {
    COMET_ASSERT(values_.find(key) != values_.cend(), "Key ", key, " (",
                 COMET_STRING_ID_LABEL(key), ") is not available!");
    return std::any_cast<T>(values_.at(key));
  }

  template <typename T, typename String>
  T Get(String&& str_key) const {
    return Get<T>(GenerateConfKey(std::forward<String>(str_key)));
  }

 private:
  std::unordered_map<ConfKey, std::any> values_;
};
}  // namespace conf
}  // namespace comet

#define COMET_CONF(type, key) \
  Engine::Get().GetConfigurationManager().Get<type>(key)
#define COMET_CONF_APP(type, key) \
  Engine::Get().GetConfigurationManager().Get<type>("application_" key)
#define COMET_CONF_CORE(type, key) \
  Engine::Get().GetConfigurationManager().Get<type>("core_" key)
#define COMET_CONF_ENTITY(type, key) \
  Engine::Get().GetConfigurationManager().Get<type>("entity_" key)
#define COMET_CONF_EVENT(type, key) \
  Engine::Get().GetConfigurationManager().Get<type>("event_" key)
#define COMET_CONF_INPUT(type, key) \
  Engine::Get().GetConfigurationManager().Get<type>("input_" key)
#define COMET_CONF_PHYSICS(type, key) \
  Engine::Get().GetConfigurationManager().Get<type>("physics_" key)
#define COMET_CONF_RENDERING(type, key) \
  Engine::Get().GetConfigurationManager().Get<type>("rendering_" key)
#define COMET_CONF_RESOURCE(type, key) \
  Engine::Get().GetConfigurationManager().Get<type>("resource_" key)
#define COMET_CONF_TIME(type, key) \
  Engine::Get().GetConfigurationManager().Get<type>("time_" key)

#endif  // COMET_COMET_CORE_CONFIGURATION_MANAGER_H_