// Copyright 2022 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_WINDOW_WINDOW_H_
#define COMET_COMET_RENDERING_WINDOW_WINDOW_H_

#include "comet_precompile.h"

namespace comet {
namespace rendering {
struct WindowDescr {
  unsigned int width = 1280;
  unsigned int height = 720;
  std::string name = "Comet Game Engine";
};

class Window {
 public:
  Window() = default;
  Window(const WindowDescr &descr);
  Window(const Window &) = default;
  Window(Window &&) = default;
  Window &operator=(const Window &) = default;
  Window &operator=(Window &&) = default;
  virtual ~Window() = default;

  virtual void Initialize() = 0;
  virtual void Destroy(){};
  virtual void Update(){};
  virtual void SetSize(unsigned int width, unsigned int height) = 0;

  virtual bool IsInitialized() const = 0;
  virtual const std::string GetName() const noexcept;
  virtual const unsigned int GetWidth() const noexcept;
  virtual const unsigned int GetHeight() const noexcept;

 protected:
  unsigned int width_ = 0;
  unsigned int height_ = 0;
  std::string name_{};
};
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_WINDOW_WINDOW_H_