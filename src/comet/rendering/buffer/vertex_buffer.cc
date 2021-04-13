// Copyright 2021 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "vertex_buffer.h"

#include "comet/core/engine.h"
#include "opengl_vertex_buffer.h"

namespace comet {
namespace rendering {
std::shared_ptr<VertexBuffer> VertexBuffer::Create(std::size_t size,
                                                   float* vertices) {
  const auto& rendering_api =
      core::Engine::GetEngine().GetRenderingManager().GetRenderingApi();

  switch (rendering_api.GetType()) {
    case RenderingApiType::kOpenGl:
      return std::make_shared<OpenGlVertexBuffer>(size, vertices);
  }

  core::Logger::Get(core::LoggerType::Rendering)
      .Error(
          "Can't create vertex buffer: current rendering API not supported.");

  return nullptr;
}
}  // namespace rendering
}  // namespace comet
