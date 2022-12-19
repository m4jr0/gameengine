// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "vulkan_shader_view.h"

namespace comet {
namespace rendering {
namespace vk {
ShaderView::ShaderView(const ShaderViewDescr& descr)
    : View{descr}, shader_handler_{descr.shader_handler} {
  COMET_ASSERT(shader_handler_ != nullptr, "Shader pass handler is null!");
}

void ShaderView::Destroy() {
  if (shader_ != nullptr && shader_handler_->IsInitialized()) {
    shader_handler_->Destroy(*shader_);
    shader_ = nullptr;
  }

  View::Destroy();
}
}  // namespace vk
}  // namespace rendering
}  // namespace comet