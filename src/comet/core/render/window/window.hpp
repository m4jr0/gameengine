// Copyright 2021 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_CORE_RENDER_WINDOW_WINDOW_HPP_
#define COMET_CORE_RENDER_WINDOW_WINDOW_HPP_

#include "comet_precompile.hpp"

namespace comet {
class Window {
 public:
  static constexpr unsigned int kDefaultWidth_ = 1280;
  static constexpr unsigned int kDefaultHeight_ = 720;
  static constexpr char kDefaultName_[] = "Comet Game Engine";

  Window(const std::string & = Window::kDefaultName_,
         unsigned int = kDefaultWidth_, unsigned int = kDefaultHeight_);
  Window(const Window &) = delete;
  Window(Window &&) = delete;
  Window &operator=(const Window &) = delete;
  Window &operator=(Window &&) = delete;
  virtual ~Window() = default;

  virtual void Initialize() = 0;
  virtual void Destroy(){};
  virtual void Update(){};
  virtual void SetSize(unsigned int, unsigned int) = 0;

  virtual const std::string name() const noexcept;
  virtual const unsigned int width() const noexcept;
  virtual const unsigned int height() const noexcept;

 protected:
  unsigned int width_ = 0;
  unsigned int height_ = 0;
  std::string name_ = kDefaultName_;
};
}  // namespace comet

#endif  // COMET_CORE_RENDER_WINDOW_WINDOW_HPP_