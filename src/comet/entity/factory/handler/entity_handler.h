// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_ENTITY_FACTORY_HANDLER_ENTITY_HANDLER_H_
#define COMET_COMET_ENTITY_FACTORY_HANDLER_ENTITY_HANDLER_H_

#include "comet_precompile.h"

#include "comet/entity/entity_manager.h"

namespace comet {
namespace entity {
struct HandlerDescr {
  EntityManager* entity_manager{nullptr};
};

class Handler {
 public:
  Handler() = delete;
  explicit Handler(const HandlerDescr& descr);
  Handler(const Handler&) = delete;
  Handler(Handler&&) = delete;
  Handler& operator=(const Handler&) = delete;
  Handler& operator=(Handler&&) = delete;
  virtual ~Handler();

  virtual void Initialize();
  virtual void Shutdown();

  bool IsInitialized() const noexcept;

 protected:
  bool is_initialized_{false};

  EntityManager* entity_manager_{nullptr};
};
}  // namespace entity
}  // namespace comet

#endif  // COMET_COMET_ENTITY_FACTORY_HANDLER_ENTITY_HANDLER_H_
