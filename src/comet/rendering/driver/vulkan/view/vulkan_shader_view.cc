// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "comet/rendering/comet_rendering_pch.h"
#include "comet_pch.h"

#include "vulkan_shader_view.h"

namespace comet {
namespace rendering {
namespace vk {
ShaderView::ShaderView(const ShaderViewDescr& descr)
    : View{descr},
      shader_handler_{descr.shader_handler},
      pipeline_handler_{descr.pipeline_handler} {
  COMET_ASSERT(shader_handler_ != nullptr, "Shader handler is null!");
  COMET_ASSERT(pipeline_handler_ != nullptr, "Pipeline handler is null!");
}

void ShaderView::Destroy() {
  if (shader_ != nullptr && shader_handler_->IsInitialized()) {
    shader_handler_->Destroy(shader_);
    shader_ = nullptr;
  }

  View::Destroy();
}
}  // namespace vk
}  // namespace rendering
}  // namespace comet