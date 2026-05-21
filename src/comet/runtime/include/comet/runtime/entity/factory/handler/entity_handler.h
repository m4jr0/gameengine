// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_RUNTIME_ENTITY_FACTORY_HANDLER_ENTITY_HANDLER_H_
#define COMET_RUNTIME_ENTITY_FACTORY_HANDLER_ENTITY_HANDLER_H_

#include "comet/core/essentials.h"

namespace comet {
namespace entity {
class Handler {
 public:
  Handler() = default;
  Handler(const Handler&) = delete;
  Handler(Handler&&) = delete;
  Handler& operator=(const Handler&) = delete;
  Handler& operator=(Handler&&) = delete;
  virtual ~Handler();

  void Initialize();
  void Shutdown();

  bool IsInitialized() const noexcept;

 protected:
  virtual void OnInitialize();
  virtual void OnShutdown();

 protected:
  bool is_initialized_{false};
};
}  // namespace entity
}  // namespace comet

#endif  // COMET_RUNTIME_ENTITY_FACTORY_HANDLER_ENTITY_HANDLER_H_
