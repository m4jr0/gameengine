// Copyright 2022 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "opengl_driver.h"

#include "comet/core/conf/configuration_manager.h"
#include "comet/core/engine.h"
#include "comet/entity/component/mesh_component.h"
#include "comet/entity/component/transform_component.h"
#include "comet/event/event_manager.h"
#include "comet/event/input_event.h"
#include "comet/event/runtime_event.h"
#include "comet/event/window_event.h"
#include "comet/rendering/driver/opengl/opengl_mesh.h"

namespace comet {
namespace rendering {
namespace gl {
OpenGlDriver::OpenGlDriver()
    : major_version_{COMET_CONF_U8(conf::kRenderingOpenGlMajorVersion)},
      minor_version_{COMET_CONF_U8(conf::kRenderingOpenGlMinorVersion)},
      clear_color_{COMET_CONF_F32(conf::kRenderingClearColorR),
                   COMET_CONF_F32(conf::kRenderingClearColorG),
                   COMET_CONF_F32(conf::kRenderingClearColorB),
                   COMET_CONF_F32(conf::kRenderingClearColorA)} {
  OpenGlGlfwWindowDescr window_descr{};
  window_descr.width =
      static_cast<WindowSize>(COMET_CONF_U16(conf::kRenderingWindowWidth));
  window_descr.height =
      static_cast<WindowSize>(COMET_CONF_U16(conf::kRenderingWindowHeight));
  window_descr.name = COMET_CONF_STR(conf::kApplicationName);
  window_descr.opengl_major_version = major_version_;
  window_descr.opengl_minor_version = minor_version_;
  window_descr.is_vsync = COMET_CONF_BOOL(conf::kRenderingIsVsync);
  window_ = OpenGlGlfwWindow(window_descr);
}

void OpenGlDriver::Initialize() {
  COMET_LOG_RENDERING_DEBUG("Initializing OpenGL driver.");
  window_.Initialize();

  if (!window_.IsInitialized()) {
    return;
  }

  auto& event_manager{Engine::Get().GetEventManager()};

  event_manager.Register(COMET_EVENT_BIND_FUNCTION(OpenGlDriver::OnEvent),
                         event::WindowResizeEvent::kStaticType_);

  const auto result{
      gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))};

  COMET_ASSERT(result, "Could not load GL Loader!");
  glEnable(GL_DEPTH_TEST);
  InitializeShader();
  is_initialized_ = true;
}

void OpenGlDriver::Destroy() { window_.Destroy(); }

void OpenGlDriver::Update(time::Interpolation interpolation,
                          entity::EntityManager& entity_manager) {
  glClearColor(clear_color_[0], clear_color_[1], clear_color_[2],
               clear_color_[3]);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // TODO(m4jr0): Remove temporary code.
  // Proxies should be managed with proper memory management and occlusion
  // culling.
  static auto view{
      entity_manager
          .GetView<entity::MeshComponent, entity::TransformComponent>()};

  for (const auto entity_id : view) {
    auto* transform_cmp{
        entity_manager.GetComponent<entity::TransformComponent>(entity_id)};

    auto* proxy{TryGetRenderProxy(entity_id)};

    if (proxy != nullptr) {
      proxy->transform += (transform_cmp->global - proxy->transform) *
                          static_cast<f32>(interpolation);
      continue;
    }

    auto* mesh_cmp{
        entity_manager.GetComponent<entity::MeshComponent>(entity_id)};

    GenerateRenderProxy(entity_id, *mesh_cmp->mesh,
                        transform_cmp->global * static_cast<f32>(interpolation),
                        *mesh_cmp->material);
  }

  DrawRenderProxies();
  window_.SwapBuffers();
}

void OpenGlDriver::SetSize(WindowSize width, WindowSize height) {
  glViewport(0, 0, window_.GetWidth(), window_.GetHeight());
}

void OpenGlDriver::OnEvent(const event::Event& event) {
  const auto& event_type{event.GetType()};

  if (event_type == event::WindowResizeEvent::kStaticType_) {
    const auto& window_event{
        static_cast<const event::WindowResizeEvent&>(event)};
    SetSize(window_event.GetWidth(), window_event.GetHeight());
  }
}

bool OpenGlDriver::IsInitialized() const { return is_initialized_; }

Window& OpenGlDriver::GetWindow() { return window_; }
}  // namespace gl
}  // namespace rendering
}  // namespace comet
