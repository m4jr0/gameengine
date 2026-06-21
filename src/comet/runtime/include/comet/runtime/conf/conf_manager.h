// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_RUNTIME_CONF_CONF_MANAGER_H_
#define COMET_RUNTIME_CONF_CONF_MANAGER_H_

#include "comet/core/container/map.h"
#include "comet/core/essentials.h"
#include "comet/core/string/tstring.h"
#include "comet/runtime/conf/config_defaults.h"
#include "comet/runtime/conf/config_keys.h"
#include "comet/runtime/conf/config_value.h"
#include "comet/runtime/manager.h"
#include "comet/runtime/memory/allocator/platform_allocator.h"

namespace comet {
namespace conf {
using ConfValues = Map<ConfKey, ConfValue>;

class ConfManager : public Manager {
 public:
  static ConfManager& Get();

  ConfManager() = default;
  ConfManager(const ConfManager&) = delete;
  ConfManager(ConfManager&&) = delete;
  ConfManager& operator=(const ConfManager&) = delete;
  ConfManager& operator=(ConfManager&&) = delete;
  ~ConfManager() override = default;

  void ParseConfFile();

  ConfValue& Get(ConfKey key);
  const ConfValue& Get(ConfKey key) const;
  const schar* GetStr(ConfKey key) const;
  const tchar* GetTStr(ConfKey key) const;
  void GetTStr(ConfKey key, TString& str) const;
  u8 GetU8(ConfKey key) const;
  u16 GetU16(ConfKey key) const;
  u32 GetU32(ConfKey key) const;
  u64 GetU64(ConfKey key) const;
  s8 GetS8(ConfKey key) const;
  s16 GetS16(ConfKey key) const;
  s32 GetS32(ConfKey key) const;
  s64 GetS64(ConfKey key) const;
  f32 GetF32(ConfKey key) const;
  f64 GetF64(ConfKey key) const;
  usize GetIndex(ConfKey key) const;
  ux GetUx(ConfKey key) const;
  sx GetSx(ConfKey key) const;
  fx GetFx(ConfKey key) const;
  bool GetBool(ConfKey key) const;

  void Set(ConfKey key, const ConfValue& value);

  void SetStr(ConfKey key, const schar* value);
  void SetStr(ConfKey key, const schar* value, usize length);
  void SetTStr(ConfKey key, const tchar* value);
  void SetTStr(ConfKey key, const tchar* value, usize length);
  void SetU8(ConfKey key, u8 value);
  void SetU16(ConfKey key, u16 value);
  void SetU32(ConfKey key, u32 value);
  void SetU64(ConfKey key, u64 value);
  void SetS8(ConfKey key, s8 value);
  void SetS16(ConfKey key, s16 value);
  void SetS32(ConfKey key, s32 value);
  void SetS64(ConfKey key, s64 value);
  void SetF32(ConfKey key, f32 value);
  void SetF64(ConfKey key, f64 value);
  void SetIndex(ConfKey key, usize value);
  void SetUx(ConfKey key, ux value);
  void SetSx(ConfKey key, sx value);
  void SetFx(ConfKey key, fx value);
  void SetBool(ConfKey key, bool value);
  void ParseKeyValuePair(schar* raw_key, usize raw_key_len, schar* value,
                         usize value_len);

 protected:
  void OnInitialize() override;
  void OnShutdown() override;

 private:
  static constexpr auto kConfigFileRelativePath_{
      COMET_CTSTRING_VIEW("./comet_config.cfg")};
  memory::PlatformAllocator allocator_{kEngineMemoryTagConfig};
  ConfValues values_{};
};
}  // namespace conf
}  // namespace comet

#define COMET_CONF(key) comet::conf::ConfManager::Get().Get(key)
#define COMET_CONF_STR(key) comet::conf::ConfManager::Get().GetStr(key)
#define COMET_CONF_TSTR(key, ...) \
  comet::conf::ConfManager::Get().GetTStr(key, ##__VA_ARGS__)
#define COMET_CONF_U8(key) comet::conf::ConfManager::Get().GetU8(key)
#define COMET_CONF_U16(key) comet::conf::ConfManager::Get().GetU16(key)
#define COMET_CONF_U32(key) comet::conf::ConfManager::Get().GetU32(key)
#define COMET_CONF_U64(key) comet::conf::ConfManager::Get().GetU64(key)
#define COMET_CONF_S8(key) comet::conf::ConfManager::Get().GetS8(key)
#define COMET_CONF_S16(key) comet::conf::ConfManager::Get().GetS16(key)
#define COMET_CONF_S32(key) comet::conf::ConfManager::Get().GetS32(key)
#define COMET_CONF_S64(key) comet::conf::ConfManager::Get().GetS64(key)
#define COMET_CONF_F32(key) comet::conf::ConfManager::Get().GetF32(key)
#define COMET_CONF_F64(key) comet::conf::ConfManager::Get().GetF64(key)
#define COMET_CONF_UINDEX(key) comet::conf::ConfManager::Get().GetIndex(key)
#define COMET_CONF_UX(key) comet::conf::ConfManager::Get().GetUx(key)
#define COMET_CONF_SX(key) comet::conf::ConfManager::Get().GetSx(key)
#define COMET_CONF_FX(key) comet::conf::ConfManager::Get().GetFx(key)
#define COMET_CONF_BOOL(key) comet::conf::ConfManager::Get().GetBool(key)

#endif  // COMET_RUNTIME_CONF_CONF_MANAGER_H_