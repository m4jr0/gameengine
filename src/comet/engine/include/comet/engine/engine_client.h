// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_ENGINE_ENGINE_CLIENT_H_
#define COMET_ENGINE_ENGINE_CLIENT_H_

#include "comet/core/essentials.h"

namespace comet {
class EngineClient {
 public:
  EngineClient() = default;
  EngineClient(const EngineClient&) = delete;
  EngineClient(EngineClient&&) = delete;
  EngineClient& operator=(const EngineClient&) = delete;
  EngineClient& operator=(EngineClient&&) = delete;
  virtual ~EngineClient() = default;

  virtual void OnInitialize() {}
  virtual void OnUpdate([[maybe_unused]] f64& lag) {}
  virtual void OnShutdown() {}
};
}  // namespace comet

#endif  // COMET_ENGINE_ENGINE_CLIENT_H_