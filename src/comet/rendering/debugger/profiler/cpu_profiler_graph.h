// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DEBUGGER_PROFILER_CPU_PROFILER_GRAPH_H_
#define COMET_COMET_RENDERING_DEBUGGER_PROFILER_CPU_PROFILER_GRAPH_H_

#include "comet/core/essentials.h"

#ifdef COMET_PROFILING
#ifdef COMET_IMGUI
#include "imgui.h"

#include "comet/rendering/debugger/profiler/cpu_profiler_displayer_context.h"

namespace comet {
namespace rendering {
class CpuProfilerGraph {
 public:
  CpuProfilerGraph();
  CpuProfilerGraph(const CpuProfilerGraph&) = delete;
  CpuProfilerGraph(CpuProfilerGraph&&) = delete;
  CpuProfilerGraph& operator=(const CpuProfilerGraph&) = delete;
  CpuProfilerGraph& operator=(CpuProfilerGraph&&) = delete;
  virtual ~CpuProfilerGraph() = default;

  void Draw(CpuProfilerDisplayerContext& context);

 private:
  struct PlottingData {
    const CpuProfilerDisplayerContext* context{nullptr};
    usize offset{0};
  };

  static inline constexpr usize kDefaultVisibleFrameCount_{1000};
  static inline constexpr usize kXLabelInterval_{100};
  static inline constexpr usize kYLabelInterval_{10};
  static inline constexpr f32 kMarkLength_{5.0f};
  static inline constexpr f32 kLabelMargin_{2.0f};
  static inline constexpr f32 kLabelTotalMargin_{kMarkLength_ + kLabelMargin_};
  static inline constexpr f32 kXLabelMargin_{50.0f};
  static inline constexpr f32 kYLabelMargin_{20.0f};
  static inline constexpr usize kGraphHeight_{150};
  static inline constexpr usize kGraphButtonsHeight_{75};
  static inline constexpr f32 kScrollZone_{.1f};
  static inline constexpr f32 kLeftScrollZone_{kScrollZone_};
  static inline constexpr f32 kRightScrollZone_{1.0f - kScrollZone_};
  static inline constexpr f32 kHighlightColorR_{220.0f};
  static inline constexpr f32 kHighlightColorG_{220.0f};
  static inline constexpr f32 kHighlightColorB_{220.0f};
  static inline constexpr f32 kHighlightColorA_{200.0f};
  static inline constexpr f32 kHighlightCursorWidth_{2.0f};
  static inline constexpr f32 kLabelColorR_{220.0f};
  static inline constexpr f32 kLabelColorG_{220.0f};
  static inline constexpr f32 kLabelColorB_{220.0f};
  static inline constexpr f32 kLabelColorA_{200.0f};
  static inline constexpr f32 kLabelMarkWidth_{1.0f};
  static inline constexpr u64 kMinScrollTimerCooldown_{100};
  static inline constexpr u64 kMaxScrollTimerCooldown_{500};
  static inline constexpr f32 kVisibleFrameScrollSpeed_{50.0f};
  static inline constexpr usize kMinVisibleFrameCount_{50};
  static inline constexpr usize kMaxVisibleFrameCount_{1500};

  static f32 GraphValueGetter(void* data, s32 relative_index);
  void DrawGraph(CpuProfilerDisplayerContext& context);
  void DrawLabels();
  void DrawXLabels();
  void DrawYLabels();
  void DrawTooltip(const CpuProfilerDisplayerContext& context,
                   usize mouse_frame_index);
  void DrawCursor(const CpuProfilerDisplayerContext& context) const;
  void DrawNavigationButtons(CpuProfilerDisplayerContext& context);
  void DrawControlButtons(CpuProfilerDisplayerContext& context);

  void HandleMouseClick(CpuProfilerDisplayerContext& context,
                        usize mouse_frame_index);
  void HandleScrollWheel(usize mouse_frame_index);
  void HandleScroll(CpuProfilerDisplayerContext& context);

  void GoToFrame(CpuProfilerDisplayerContext& context, usize frame_index,
                 bool is_scroll = true);
  void UpdateCursorPos(const CpuProfilerDisplayerContext& context);
  void ToggleFocusMode(CpuProfilerDisplayerContext& context);

  f32 max_y_value_{.0f};

  ImVec2 graph_origin_{};
  ImVec2 graph_size_{};
  ImVec2 local_graph_mouse_pos_{};
  ImVec2 local_graph_cursor_pos_{};

  usize visible_frame_count_{kDefaultVisibleFrameCount_};
  usize scroll_amount_{visible_frame_count_ / 4};
  usize start_frame_index_{0};
  usize end_frame_index_{0};
  usize max_visible_frame_index_{0};
  usize scroll_timer_{0};
  u64 scroll_timer_cooldown_{kMaxScrollTimerCooldown_};

  ImDrawList* draw_list_{nullptr};
};
}  // namespace rendering
}  // namespace comet
#endif  // COMET_IMGUI
#endif  // COMET_PROFILING

#endif  // COMET_COMET_RENDERING_DEBUGGER_PROFILER_CPU_PROFILER_GRAPH_H_
