// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_EMPTY_EMPTY_DRIVER_H_
#define COMET_COMET_RENDERING_DRIVER_EMPTY_EMPTY_DRIVER_H_

#ifdef COMET_DEBUG

#include "comet/core/essentials.h"
#include "comet/core/frame/frame_packet.h"
#include "comet/core/memory/memory.h"
#include "comet/rendering/driver/driver.h"
#include "comet/rendering/window/glfw/empty/empty_glfw_window.h"

namespace comet {
namespace rendering {
namespace empty {
struct EmptyDriverDescr : DriverDescr {};

class EmptyDriver : public Driver {
 public:
  explicit EmptyDriver(const EmptyDriverDescr& descr);
  EmptyDriver(const EmptyDriver&) = delete;
  EmptyDriver(EmptyDriver&&) = delete;
  EmptyDriver& operator=(const EmptyDriver&) = delete;
  EmptyDriver& operator=(EmptyDriver&&) = delete;
  virtual ~EmptyDriver() = default;

  void Initialize() override;
  void Shutdown() override;
  void Update(frame::FramePacket*) override;
  DriverType GetType() const noexcept override;

  void SetSize(WindowSize width, WindowSize height);
  Window* GetWindow() override;
  u32 GetDrawCount() const override;

 private:
  memory::UniquePtr<EmptyGlfwWindow> window_{nullptr};
};
}  // namespace empty
}  // namespace rendering
}  // namespace comet

#endif  // COMET_DEBUG

#endif  // COMET_COMET_RENDERING_DRIVER_EMPTY_EMPTY_DRIVER_H_
