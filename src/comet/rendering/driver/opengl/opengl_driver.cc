// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "opengl_driver.h"

#include "comet/event/input_event.h"
#include "comet/event/runtime_event.h"
#include "comet/event/window_event.h"
#include "comet/physics/component/transform_component.h"
#include "comet/rendering/driver/opengl/view/opengl_view.h"
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

  if (is_sampler_anisotropy_) {
#ifdef GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT
    GLfloat max_anisotropy;
    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &max_anisotropy);
    glSamplerParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT,
                        max_anisotropy);
#else
    COMET_LOG_RENDERING_ERROR(
        "Sampler anisotropy is enabled, but current driver does not seem to "
        "support it. Ignoring feature.");
#endif  // GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT
  }

  InitializeHandlers();
}

void OpenGlDriver::Shutdown() {
  DestroyHandlers();
  window_->Destroy();
  frame_count_ = 0;
  Driver::Shutdown();
}

void OpenGlDriver::Update(time::Interpolation interpolation) {
  glClearColor(clear_color_[0], clear_color_[1], clear_color_[2],
               clear_color_[3]);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  Draw(interpolation);
  window_->SwapBuffers();
  ++frame_count_;
}

DriverType OpenGlDriver::GetType() const noexcept { return DriverType::OpenGl; }

u32 OpenGlDriver::GetDrawCount() const {
  return render_proxy_handler_->GetDrawCount();
}

void OpenGlDriver::InitializeHandlers() {
  MeshHandlerDescr mesh_handler_descr{};
  mesh_handler_ = std::make_unique<MeshHandler>(mesh_handler_descr);

  ShaderModuleHandlerDescr shader_module_handler_descr{};
  shader_module_handler_descr.resource_manager = resource_manager_;
  shader_module_handler_ =
      std::make_unique<ShaderModuleHandler>(shader_module_handler_descr);

  TextureHandlerDescr texture_handler_descr{};
  texture_handler_ = std::make_unique<TextureHandler>(texture_handler_descr);

  ShaderHandlerDescr shader_handler_descr{};
  shader_handler_descr.resource_manager = resource_manager_;
  shader_handler_descr.shader_module_handler = shader_module_handler_.get();
  shader_handler_descr.texture_handler = texture_handler_.get();
  shader_handler_ = std::make_unique<ShaderHandler>(shader_handler_descr);

  MaterialHandlerDescr material_handler_descr{};
  material_handler_descr.resource_manager = resource_manager_;
  material_handler_descr.shader_handler = shader_handler_.get();
  material_handler_descr.texture_handler = texture_handler_.get();
  material_handler_ = std::make_unique<MaterialHandler>(material_handler_descr);

  RenderProxyHandlerDescr render_proxy_handler_descr{};
  render_proxy_handler_descr.material_handler = material_handler_.get();
  render_proxy_handler_descr.mesh_handler = mesh_handler_.get();
  render_proxy_handler_descr.shader_handler = shader_handler_.get();
  render_proxy_handler_descr.camera_manager = camera_manager_;
  render_proxy_handler_descr.entity_manager = entity_manager_;
  render_proxy_handler_ =
      std::make_unique<RenderProxyHandler>(render_proxy_handler_descr);

  ViewHandlerDescr view_handler_descr{};
  view_handler_descr.shader_handler = shader_handler_.get();
  view_handler_descr.render_proxy_handler = render_proxy_handler_.get();
#ifdef COMET_DEBUG
  view_handler_descr.debugger_displayer_manager = debugger_displayer_manager_;
#endif  // COMET_DEBUG
  view_handler_descr.rendering_view_descrs = &rendering_view_descrs_;
  view_handler_descr.window = window_.get();
  view_handler_ = std::make_unique<ViewHandler>(view_handler_descr);

  texture_handler_->Initialize();
  shader_module_handler_->Initialize();
  shader_handler_->Initialize();
  material_handler_->Initialize();
  mesh_handler_->Initialize();
  render_proxy_handler_->Initialize();
  view_handler_->Initialize();
}

void OpenGlDriver::DestroyHandlers() {
  // Order is important to improve performance.
  shader_module_handler_->Shutdown();
  shader_handler_->Shutdown();
  texture_handler_->Shutdown();
  material_handler_->Shutdown();
  mesh_handler_->Shutdown();
  render_proxy_handler_->Shutdown();
  view_handler_->Shutdown();

  shader_module_handler_ = nullptr;
  shader_handler_ = nullptr;
  texture_handler_ = nullptr;
  material_handler_ = nullptr;
  mesh_handler_ = nullptr;
  render_proxy_handler_ = nullptr;
  view_handler_ = nullptr;
}

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

void OpenGlDriver::Draw(time::Interpolation interpolation) {
  ViewPacket packet{};
  packet.frame_count = frame_count_;
  packet.interpolation = interpolation;

  auto* camera{camera_manager_->GetMainCamera()};
  packet.projection_matrix = camera->GetProjectionMatrix();
  packet.view_matrix = &camera->GetViewMatrix();
  view_handler_->Update(packet);
}
}  // namespace gl
}  // namespace rendering
}  // namespace comet
