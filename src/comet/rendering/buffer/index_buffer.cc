// Copyright 2021 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "index_buffer.h"

#include "comet/core/engine.h"
#include "opengl_index_buffer.h"

namespace comet {
namespace rendering {
std::shared_ptr<IndexBuffer> IndexBuffer::Create(std::size_t size,
                                                 unsigned int* indices) {
  const auto& rendering_api =
      core::Engine::GetEngine().GetRenderingManager().GetRenderingApi();

  switch (rendering_api.GetType()) {
    case RenderingApiType::kOpenGl:
      return std::make_shared<OpenGlIndexBuffer>(size, indices);
  }

  core::Logger::Get(core::LoggerType::Rendering)
      .Error("Can't create index buffer: current rendering API not supported.");

  return nullptr;
}
}  // namespace rendering
}  // namespace comet
