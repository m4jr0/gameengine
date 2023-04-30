// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "opengl_driver.h"

#include "comet/event/input_event.h"
#include "comet/event/runtime_event.h"
#include "comet/event/window_event.h"
#include "comet/physics/component/transform_component.h"
#include "comet/resource/component/mesh_component.h"

namespace comet {
namespace rendering {
namespace gl {
OpenGlDriver::OpenGlDriver(const OpenGlDriverDescr& descr) : Driver(descr) {
  OpenGlGlfwWindowDescr window_descr{};
  window_descr.width = window_width_;
  window_descr.height = window_height_;
  window_descr.name = app_name_;
  window_descr.opengl_major_version = descr.opengl_major_version;
  window_descr.opengl_minor_version = descr.opengl_minor_version;
  window_descr.is_vsync = is_vsync_;
  window_descr.anti_aliasing_type = anti_aliasing_type_;
  window_descr.event_manager = event_manager_;
  window_ = std::make_unique<OpenGlGlfwWindow>(window_descr);

  if (is_triple_buffering_) {
    COMET_LOG_RENDERING_WARNING(
        "Triple buffering is not currently supported on the OpenGL driver. "
        "Disabling it.");
    is_triple_buffering_ = false;
  }
}

void OpenGlDriver::Initialize() {
  Driver::Initialize();
  COMET_LOG_RENDERING_DEBUG("Initializing OpenGL driver.");
  window_->Initialize();
  COMET_ASSERT(window_->IsInitialized(), "Window could not be initialized!");

  event_manager_->Register(COMET_EVENT_BIND_FUNCTION(OpenGlDriver::OnEvent),
                           event::WindowResizeEvent::kStaticType_);

  const auto result{
      gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))};

  COMET_ASSERT(result, "Could not load GL Loader!");
  glEnable(GL_DEPTH_TEST);

  if (anti_aliasing_type_ != AntiAliasingType::None) {
    glEnable(GL_MULTISAMPLE);

    if (is_sample_rate_shading_) {
      glEnable(GL_SAMPLE_SHADING);
      glMinSampleShading(.2f);
    }
  }

  TemporaryHandlerDescr tmp_handler_descr{};
  tmp_handler_descr.camera_manager = camera_manager_;
  tmp_handler_descr.debugger_displayer_manager = debugger_displayer_manager_;
  tmp_handler_descr.resource_manager = resource_manager_;
  tmp_handler_descr.window = window_.get();
  tmp_handler_ = std::make_unique<TemporaryHandler>(tmp_handler_descr);
  tmp_handler_->Initialize();
}

void OpenGlDriver::Shutdown() {
  tmp_handler_->Shutdown();
  tmp_handler_ = nullptr;
  window_->Destroy();
  Driver::Shutdown();
}

void OpenGlDriver::Update(time::Interpolation interpolation) {
  // Temporary code to display something in OpenGL.
  // This code is CERTAINLY not final. For now, it only shows that both OpenGL
  // and Vulkan can work independently.

  glClearColor(clear_color_[0], clear_color_[1], clear_color_[2],
               clear_color_[3]);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // TODO(m4jr0): Remove temporary code.
  // Proxies should be managed with proper memory management and occlusion
  // culling.
  entity_manager_->Each<resource::MeshComponent, physics::TransformComponent>(
      [&](auto entity_id) {
        auto* transform_cmp{
            entity_manager_->GetComponent<physics::TransformComponent>(
                entity_id)};

        auto* proxy{tmp_handler_->TryGetRenderProxy(entity_id)};

        if (proxy != nullptr) {
          proxy->transform += (transform_cmp->global - proxy->transform) *
                              static_cast<f32>(interpolation);
          return;
        }

        auto* mesh_cmp{
            entity_manager_->GetComponent<resource::MeshComponent>(entity_id)};

        tmp_handler_->GenerateRenderProxy(
            entity_id, *mesh_cmp->mesh,
            transform_cmp->global * static_cast<f32>(interpolation),
            *mesh_cmp->material, is_sampler_anisotropy_);
      });

  tmp_handler_->DrawRenderProxies();
  tmp_handler_->DrawUi();
  window_->SwapBuffers();
}

DriverType OpenGlDriver::GetType() const noexcept { return DriverType::OpenGl; }

u32 OpenGlDriver::GetDrawCount() const { return tmp_handler_->GetDrawCount(); }

void OpenGlDriver::SetSize(WindowSize width, WindowSize height) {
  glViewport(0, 0, window_->GetWidth(), window_->GetHeight());
}

void OpenGlDriver::OnEvent(const event::Event& event) {
  const auto& event_type{event.GetType()};

  if (event_type == event::WindowResizeEvent::kStaticType_) {
    const auto& window_resize_event{
        static_cast<const event::WindowResizeEvent&>(event)};
    SetSize(window_resize_event.GetWidth(), window_resize_event.GetHeight());
  }
}

Window* OpenGlDriver::GetWindow() { return window_.get(); }
}  // namespace gl
}  // namespace rendering
}  // namespace comet
