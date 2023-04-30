// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_VULKAN_VIEW_VULKAN_VIEW_H_
#define COMET_COMET_RENDERING_DRIVER_VULKAN_VIEW_VULKAN_VIEW_H_

#include "comet_precompile.h"

#include "vulkan/vulkan.h"

#include "comet/math/matrix.h"
#include "comet/rendering/driver/vulkan/data/vulkan_frame.h"
#include "comet/rendering/driver/vulkan/data/vulkan_render_pass.h"
#include "comet/rendering/driver/vulkan/handler/vulkan_render_pass_handler.h"
#include "comet/rendering/driver/vulkan/vulkan_context.h"
#include "comet/rendering/rendering_common.h"
#include "comet/time/time_manager.h"

#ifdef COMET_DEBUG
#include "comet/rendering/debugger/debugger_displayer_manager.h"
#endif  // COMET_DEBUG

namespace comet {
namespace rendering {
namespace vk {
struct ViewPacket {
  time::Interpolation interpolation{1};
  FrameInFlightIndex frame_in_flight_index{kInvalidFrameInFlightIndex};
  VkCommandBuffer command_buffer_handle{VK_NULL_HANDLE};
  math::Mat4 projection_matrix{1.0f};
  const math::Mat4* view_matrix{nullptr};
};

struct ViewDescr {
  bool is_first{false};
  bool is_last{false};
  WindowSize width{0};
  WindowSize height{0};
  f32 clear_color[4]{kColorBlack[0], kColorBlack[1], kColorBlack[2], 1.0f};
  RenderingViewId id{kInvalidRenderingViewId};
  const Context* context{nullptr};
  RenderPassHandler* render_pass_handler{nullptr};
};

class View {
 public:
  explicit View(const ViewDescr& descr);
  View(const View&) = delete;
  View(View&&) = delete;
  View& operator=(const View&) = delete;
  View& operator=(View&&) = delete;
  virtual ~View();

  virtual void Initialize();
  virtual void Destroy();
  virtual void Update(const ViewPacket& packet) = 0;
  virtual void SetSize(WindowSize width, WindowSize height);

  bool IsInitialized() const noexcept;
  RenderingViewId GetId() const noexcept;

 protected:
  bool is_initialized_{false};
  bool is_first_{false};
  bool is_last_{false};
  WindowSize width_{0};
  WindowSize height_{0};
  f32 clear_color_[4]{kColorBlack[0], kColorBlack[1], kColorBlack[2], 1.0f};
  RenderingViewId id_{kInvalidRenderingViewId};
  const Context* context_{nullptr};
  RenderPass* render_pass_{nullptr};
  RenderPassHandler* render_pass_handler_{nullptr};
};
}  // namespace vk
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_VULKAN_VIEW_VULKAN_VIEW_H_
