// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_ANIMATION_ANIMATION_MANAGER_H_
#define COMET_COMET_ANIMATION_ANIMATION_MANAGER_H_

#include "comet/core/concurrency/job/job.h"
#include "comet/core/essentials.h"
#include "comet/core/frame/frame_packet.h"
#include "comet/core/manager.h"

namespace comet {
namespace animation {
class AnimationManager : public Manager {
 public:
  static AnimationManager& Get();
  void Update(frame::FramePacket* packet);

  AnimationManager() = default;
  AnimationManager(const AnimationManager&) = delete;
  AnimationManager(AnimationManager&&) = delete;
  AnimationManager& operator=(const AnimationManager&) = delete;
  AnimationManager& operator=(AnimationManager&&) = delete;
  virtual ~AnimationManager() = default;

 private:
  static void OnAssetProcessing(job::JobParamsHandle params_handle);
};
}  // namespace animation
}  // namespace comet

#endif  // COMET_COMET_ANIMATION_ANIMATION_MANAGER_H_
