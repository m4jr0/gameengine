// Copyright 2022 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "window.h"

namespace comet {
namespace rendering {
Window::Window(const std::string& name, unsigned int width,
               unsigned int height) {
  name_ = name;
  width_ = width;
  height_ = height;
}

const std::string Window::GetName() const noexcept { return name_; }

const unsigned int Window::GetWidth() const noexcept { return width_; }

const unsigned int Window::GetHeight() const noexcept { return height_; }
}  // namespace rendering
}  // namespace comet
