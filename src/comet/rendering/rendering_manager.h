// Copyright 2022 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_RENDERING_MANAGER_H_
#define COMET_COMET_RENDERING_RENDERING_MANAGER_H_

#include "comet/core/manager.h"
#include "comet/game_object/game_object_manager.h"
#include "comet/rendering/driver/driver.h"
#include "comet/time/time_manager.h"
#include "comet_precompile.h"

namespace comet {
namespace rendering {
class RenderingManager : public core::Manager {
 public:
  RenderingManager() = default;
  RenderingManager(const RenderingManager&) = delete;
  RenderingManager(RenderingManager&&) = delete;
  RenderingManager& operator=(const RenderingManager&) = delete;
  RenderingManager& operator=(RenderingManager&&) = delete;
  virtual ~RenderingManager() = default;

  void Initialize() override;
  void Destroy() override;
  void Update(time::Interpolation interpolation,
              game_object::GameObjectManager& game_object_manager);

  const Window* GetWindow() const;

 private:
  int counter_ = 0;
  double current_time_ = 0;
  std::unique_ptr<Driver> driver_ = nullptr;

  void GenerateOpenGlDriver();
  void GenerateVulkanDriver();
  void GenerateDirect3D12Driver();
};
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_RENDERING_MANAGER_H_
