// Copyright 2021 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_RENDERING_API_H_
#define COMET_COMET_RENDERING_RENDERING_API_H_

#include "comet_precompile.h"

namespace comet {
namespace rendering {
enum class RenderingApiType { kNone = 0, kOpenGl, kDirectX12, kVulkan };

class RenderingApi {
 public:
  RenderingApi() = default;
  RenderingApi(const RenderingApi&) = delete;
  RenderingApi(RenderingApi&&) = delete;
  RenderingApi& operator=(const RenderingApi&) = delete;
  RenderingApi& operator=(RenderingApi&&) = delete;
  virtual ~RenderingApi() = default;

  virtual RenderingApiType GetType() const noexcept = 0;
};
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_RENDERING_API_H_
