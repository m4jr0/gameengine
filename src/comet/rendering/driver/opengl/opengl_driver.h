// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_OPENGL_OPENGL_DRIVER_H_
#define COMET_COMET_RENDERING_DRIVER_OPENGL_OPENGL_DRIVER_H_

#include "comet_precompile.h"

#include "glad/glad.h"

#include "comet/event/event.h"
#include "comet/event/event_manager.h"
#include "comet/rendering/driver/driver.h"
#include "comet/rendering/driver/opengl/opengl_temporary_handler.h"
#include "comet/rendering/rendering_common.h"
#include "comet/rendering/window/glfw/opengl/opengl_glfw_window.h"

namespace comet {
namespace rendering {
namespace gl {
struct OpenGlDriverDescr : DriverDescr {
  u8 opengl_major_version{0};
  u8 opengl_minor_version{0};
};

class OpenGlDriver : public Driver {
 public:
  explicit OpenGlDriver(const OpenGlDriverDescr& descr);
  OpenGlDriver(const OpenGlDriver&) = delete;
  OpenGlDriver(OpenGlDriver&&) = delete;
  OpenGlDriver& operator=(const OpenGlDriver&) = delete;
  OpenGlDriver& operator=(OpenGlDriver&&) = delete;
  virtual ~OpenGlDriver() = default;

  void Initialize() override;
  void Shutdown() override;
  void Update(time::Interpolation interpolation) override;
  DriverType GetType() const noexcept override;
  u32 GetDrawCount() const override;

  void SetSize(WindowSize width, WindowSize height);
  void OnEvent(const event::Event&);

  Window* GetWindow() override;

 private:
  std::unique_ptr<OpenGlGlfwWindow> window_{nullptr};
  std::unique_ptr<TemporaryHandler> tmp_handler_{nullptr};
};
}  // namespace gl
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_OPENGL_OPENGL_DRIVER_H_
