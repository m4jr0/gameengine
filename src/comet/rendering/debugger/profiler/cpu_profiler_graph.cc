// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "comet/rendering/comet_rendering_pch.h"
#include "comet_pch.h"

#include "cpu_profiler_graph.h"

#ifdef COMET_PROFILING
#ifdef COMET_IMGUI
#include "comet/core/c_string.h"
#include "comet/core/date.h"
#include "comet/core/game_state_manager.h"
#include "comet/math/math_common.h"
#include "comet/profiler/profiler_manager.h"

namespace comet {
namespace rendering {
CpuProfilerGraph::CpuProfilerGraph()
    : max_y_value_{
          static_cast<f32>(time::TimeManager::Get().GetFixedDeltaTime() * 2)} {}

void CpuProfilerGraph::Draw(CpuProfilerDisplayerContext& context) {
  draw_list_ = ImGui::GetWindowDrawList();
  ImGui::Text("Frame Time Graph (ms)");
  DrawGraph(context);
  UpdateCursorPos(context);
  DrawCursor(context);
}

f32 CpuProfilerGraph::GraphValueGetter(void* data, s32 relative_index) {
  auto* plotting_data{reinterpret_cast<PlottingData*>(data)};
  const auto* frame_context{plotting_data->context->GetFrameContext(
      plotting_data->offset + relative_index)};

  if (frame_context == nullptr) {
    return .0f;
  }

  return frame_context->elapsed_time_ms;
}

void CpuProfilerGraph::DrawGraph(CpuProfilerDisplayerContext& context) {
  const auto& frame_contexts{*context.frame_contexts};

  if (frame_contexts.IsEmpty()) {
    ImGui::Text("No recorded data to visualize.");
    return;
  }

  auto frame_context_count{frame_contexts.GetSize()};

  if (!context.is_frame_focused) {
    if (frame_context_count > visible_frame_count_) {
      start_frame_index_ = frame_context_count - visible_frame_count_;
    } else {
      start_frame_index_ = 0;
    }
  }

  max_visible_frame_index_ = start_frame_index_ + visible_frame_count_;
  end_frame_index_ = math::Min(max_visible_frame_index_, frame_context_count);

  if (ImGui::BeginChild("FrameGraph",
                        ImVec2{0, kGraphHeight_ + kGraphButtonsHeight_},
                        ImGuiChildFlags_AutoResizeY)) {
    auto child_size{ImGui::GetContentRegionAvail()};

    PlottingData plotting_data{};
    plotting_data.context = &context;
    plotting_data.offset = start_frame_index_;

    auto cursor_pos = ImGui::GetCursorScreenPos();
    ImGui::SetCursorScreenPos(
        ImVec2{cursor_pos.x + kYLabelMargin_, cursor_pos.y});

    ImGui::PlotLines("##Frame Times", GraphValueGetter, &plotting_data,
                     static_cast<s32>(visible_frame_count_), 0, nullptr, 0.0f,
                     max_y_value_,
                     ImVec2{child_size.x - kYLabelMargin_, kGraphHeight_});
    graph_origin_ = ImGui::GetItemRectMin();
    graph_size_ = ImGui::GetItemRectSize();
    auto mouse_pos{ImGui::GetMousePos()};
    local_graph_mouse_pos_ =
        ImVec2{mouse_pos.x - graph_origin_.x, mouse_pos.y - graph_origin_.y};

    DrawLabels();

    auto is_hover{ImGui::IsItemHovered()};
    // Somehow, sometimes, ImGui::IsItemActive() returns false?
    auto is_left_mouse{is_hover && ImGui::IsMouseDown(ImGuiMouseButton_Left)};

    if (is_left_mouse || is_hover) {
      auto mouse_frame_index{
          static_cast<usize>(local_graph_mouse_pos_.x /
                             (graph_size_.x / visible_frame_count_)) +
          start_frame_index_};

      if (is_left_mouse) {
        HandleMouseClick(context, mouse_frame_index);
      }

      if (is_hover) {
        HandleScrollWheel(mouse_frame_index);
        DrawTooltip(context, mouse_frame_index);
      }
    }
  }

  ImGui::Spacing();
  ImGui::NewLine();

  DrawNavigationButtons(context);
  DrawControlButtons(context);

  ImGui::EndChild();
}

void CpuProfilerGraph::DrawLabels() {
  DrawXLabels();
  DrawYLabels();
}

void CpuProfilerGraph::DrawXLabels() {
  constexpr usize kLabelLen{63};
  schar label[kLabelLen + 1]{'\0'};

  // Adapt interval based on zoom level.
  auto x_label_interval{kXLabelInterval_ * visible_frame_count_ /
                        kDefaultVisibleFrameCount_};

  auto first_frame_label{(start_frame_index_ + x_label_interval - 1) /
                         x_label_interval * x_label_interval};

  // Track last label end position.
  auto prev_label_anchor{kF32Min};

  for (usize frame{first_frame_label}; frame < max_visible_frame_index_;
       frame += x_label_interval) {
    auto t{static_cast<f32>(frame - start_frame_index_) / visible_frame_count_};
    auto x{graph_origin_.x + t * graph_size_.x};

    draw_list_->AddLine(
        ImVec2{x, graph_origin_.y + graph_size_.y},
        ImVec2{x, graph_origin_.y + graph_size_.y + kMarkLength_},
        IM_COL32(kLabelColorR_, kLabelColorG_, kLabelColorB_, kLabelColorA_),
        kLabelMarkWidth_);

    ConvertToStr(frame, label, kLabelLen);
    auto label_size{ImGui::CalcTextSize(label)};

    if (x - label_size.x * .5f <= prev_label_anchor) {
      // Ignoring label (it would overlaps the previous one).
      continue;
    }

    draw_list_->AddText(
        ImVec2{x - label_size.x * .5f,
               graph_origin_.y + graph_size_.y + kLabelTotalMargin_},
        IM_COL32(kLabelColorR_, kLabelColorG_, kLabelColorB_, kLabelColorA_),
        label);

    prev_label_anchor = x + label_size.x * .5f;
  }
}

void CpuProfilerGraph::DrawYLabels() {
  constexpr usize kLabelLen{7};
  schar label[kLabelLen + 1]{'\0'};

  for (usize value{0}; value <= static_cast<usize>(max_y_value_);
       value += kYLabelInterval_) {
    auto t{static_cast<f32>(value) / max_y_value_};
    auto y{graph_origin_.y + graph_size_.y - t * graph_size_.y};

    draw_list_->AddLine(
        ImVec2{graph_origin_.x - kMarkLength_, y}, ImVec2{graph_origin_.x, y},
        IM_COL32(kLabelColorR_, kLabelColorG_, kLabelColorB_, kLabelColorA_),
        kLabelMarkWidth_);

    ConvertToStr(value, label, kLabelLen);
    auto label_size{ImGui::CalcTextSize(label)};

    draw_list_->AddText(
        ImVec2{graph_origin_.x - label_size.x - kLabelTotalMargin_,
               y - label_size.y * 0.5f},
        IM_COL32(kLabelColorR_, kLabelColorG_, kLabelColorB_, kLabelColorA_),
        label);
  }
}

void CpuProfilerGraph::DrawTooltip(const CpuProfilerDisplayerContext& context,
                                   usize mouse_frame_index) {
  const auto* frame_context{context.GetFrameContext(mouse_frame_index)};

  if (frame_context == nullptr) {
    ImGui::SetTooltip("No record available");
    return;
  }

  ImGui::SetTooltip("Frame: %zu\nTime: %.2f ms", frame_context->frame_count,
                    frame_context->elapsed_time_ms);
}

void CpuProfilerGraph::DrawCursor(
    const CpuProfilerDisplayerContext& context) const {
  if (!context.is_frame_focused) {
    return;
  }

  auto cursor_end{local_graph_cursor_pos_};
  cursor_end.y += kGraphHeight_;

  ImGui::GetWindowDrawList()->AddLine(
      local_graph_cursor_pos_, cursor_end,
      IM_COL32(kHighlightColorR_, kHighlightColorG_, kHighlightColorB_,
               kHighlightColorA_),
      kHighlightCursorWidth_);
}

void CpuProfilerGraph::DrawNavigationButtons(
    CpuProfilerDisplayerContext& context) {
  constexpr auto kButtonWidth{50.0f};
  constexpr auto kButtonHeight{20.0f};
  constexpr auto kButtonCount{6};
  const auto kButtonSpacing{ImGui::GetStyle().ItemSpacing.x};

  auto total_width{kButtonCount * kButtonWidth +
                   (kButtonCount - 1) * kButtonSpacing};

  auto window_width{ImGui::GetWindowSize().x};
  auto start_x{(window_width - total_width) / 2};
  ImGui::SetCursorPosX(start_x);
  auto frame_index{context.GetFrameIndex()};

  if (ImGui::Button("<<<", ImVec2{kButtonWidth, kButtonHeight})) {
    GoToFrame(context, 0);
  }

  ImGui::SameLine();

  if (ImGui::Button("<<", ImVec2{kButtonWidth, kButtonHeight})) {
    GoToFrame(context,
              frame_index >= scroll_amount_ ? frame_index - scroll_amount_ : 0);
  }

  ImGui::SameLine();

  if (ImGui::Button("<", ImVec2{kButtonWidth, kButtonHeight})) {
    GoToFrame(context, frame_index >= 1 ? frame_index - 1 : 0);
  }

  ImGui::SameLine();

  if (ImGui::Button(">", ImVec2{kButtonWidth, kButtonHeight})) {
    GoToFrame(context, frame_index + 1);
  }

  ImGui::SameLine();

  if (ImGui::Button(">>", ImVec2{kButtonWidth, kButtonHeight})) {
    GoToFrame(context, frame_index + scroll_amount_);
  }

  ImGui::SameLine();

  if (ImGui::Button(">>>", ImVec2{kButtonWidth, kButtonHeight})) {
    GoToFrame(context, context.frame_contexts->GetSize() - 1);
  }
}

void CpuProfilerGraph::DrawControlButtons(
    CpuProfilerDisplayerContext& context) {
  constexpr auto kButtonWidth{75.0f};
  constexpr auto kButtonHeight{20.0f};
  constexpr auto kButtonCount{3};
  const auto kButtonSpacing{ImGui::GetStyle().ItemSpacing.x};

  auto total_width{kButtonCount * kButtonWidth +
                   (kButtonCount - 1) * kButtonSpacing};

  auto window_width{ImGui::GetWindowSize().x};
  auto start_x{(window_width - total_width) / 2};
  ImGui::SetCursorPosX(start_x);

  auto& profiler_manager{profiler::ProfilerManager::Get()};
  auto is_recording{profiler_manager.IsRecording()};

  if (ImGui::Button(is_recording ? "Stop" : "Record",
                    ImVec2{kButtonWidth, kButtonHeight})) {
    profiler_manager.ToggleRecording();
  }

  ImGui::SameLine();

  if (ImGui::Button(GameStateManager::Get().IsPaused() ? "Play" : "Pause",
                    ImVec2{kButtonWidth, kButtonHeight})) {
    GameStateManager::Get().TogglePause();
  }

  ImGui::SameLine();

  if (ImGui::Button(context.is_frame_focused ? "Unfocus" : "Focus",
                    ImVec2{kButtonWidth, kButtonHeight})) {
    ToggleFocusMode(context);
  }
}

void CpuProfilerGraph::HandleMouseClick(CpuProfilerDisplayerContext& context,
                                        usize mouse_frame_index) {
  auto now{GetTimestampMilliSeconds()};
  auto is_scroll{now - scroll_timer_ > scroll_timer_cooldown_};

  if (is_scroll) {
    scroll_timer_ = now;
  }

  auto old_start_frame_index{start_frame_index_};

  GoToFrame(context, mouse_frame_index, is_scroll);

  // Update focus position to prevent flickering cursor.
  if (is_scroll && context.is_frame_focused) {
    auto sign{
        static_cast<s8>(start_frame_index_ > old_start_frame_index ? 1 : -1)};
    auto delta{sign >= 0 ? start_frame_index_ - old_start_frame_index
                         : old_start_frame_index - start_frame_index_};
    context.Focus(context.GetFrameIndex() + sign * static_cast<s64>(delta));
  }
}

void CpuProfilerGraph::HandleScrollWheel(usize mouse_frame_index) {
  auto scroll_amount{ImGui::GetIO().MouseWheel};

  if (scroll_amount == .0f) {
    return;
  }

  scroll_amount = -scroll_amount;

  auto delta{
      static_cast<usize>(math::Abs(scroll_amount) * kVisibleFrameScrollSpeed_)};
  auto old_visible_frame_count{visible_frame_count_};

  if (scroll_amount >= .0f) {
    visible_frame_count_ =
        math::Min(visible_frame_count_ + delta, kMaxVisibleFrameCount_);
  } else {
    if (delta >= visible_frame_count_) {
      visible_frame_count_ = kMinVisibleFrameCount_;
    } else {
      visible_frame_count_ =
          math::Max(visible_frame_count_ - delta, kMinVisibleFrameCount_);
    }
  }

  auto current_delta{static_cast<f32>(mouse_frame_index - start_frame_index_) /
                     static_cast<f32>(old_visible_frame_count)};
  start_frame_index_ = static_cast<usize>(
      -current_delta * visible_frame_count_ + mouse_frame_index);
}

void CpuProfilerGraph::HandleScroll(CpuProfilerDisplayerContext& context) {
  auto frame_context_count{context.frame_contexts->GetSize()};

  if (frame_context_count <= visible_frame_count_) {
    return;
  }

  auto frame_index{context.GetFrameIndex()};

  if (frame_index < start_frame_index_) {
    start_frame_index_ =
        frame_index > scroll_amount_ ? frame_index - scroll_amount_ : 0;
    return;
  }

  auto percentage{static_cast<f32>(frame_index - start_frame_index_) /
                  visible_frame_count_};

  if (percentage <= kLeftScrollZone_ && start_frame_index_ > 0) {
    scroll_timer_cooldown_ = math::Clamp(
        static_cast<u64>(static_cast<f32>(kMaxScrollTimerCooldown_) *
                         (1 - (kLeftScrollZone_ - percentage) / kScrollZone_)),
        kMinScrollTimerCooldown_, kMaxScrollTimerCooldown_);

    start_frame_index_ =
        frame_index >= scroll_amount_ ? frame_index - scroll_amount_ : 0;
  } else if (percentage > kRightScrollZone_ &&
             end_frame_index_ < frame_context_count) {
    scroll_timer_cooldown_ = math::Clamp(
        static_cast<u64>(static_cast<f32>(kMaxScrollTimerCooldown_) *
                         (1 - (percentage - kRightScrollZone_) / kScrollZone_)),
        kMinScrollTimerCooldown_, kMaxScrollTimerCooldown_);

    start_frame_index_ =
        math::Min(frame_index + scroll_amount_ - visible_frame_count_,
                  frame_context_count - visible_frame_count_);
  }
}

void CpuProfilerGraph::GoToFrame(CpuProfilerDisplayerContext& context,
                                 usize frame_index, bool is_scroll) {
  auto frame_context_count{context.frame_contexts->GetSize()};

  if (frame_context_count == 1) {
    context.Focus(0);
    return;
  }

  context.Focus(frame_index);

  if (is_scroll) {
    HandleScroll(context);
  }
}

void CpuProfilerGraph::UpdateCursorPos(
    const CpuProfilerDisplayerContext& context) {
  auto cursor_index{context.GetFrameIndex() - start_frame_index_};
  auto cursor_x{graph_size_.x *
                (static_cast<f32>(cursor_index) / visible_frame_count_)};
  local_graph_cursor_pos_.x = graph_origin_.x + cursor_x;
  local_graph_cursor_pos_.y = graph_origin_.y;
}

void CpuProfilerGraph::ToggleFocusMode(CpuProfilerDisplayerContext& context) {
  if (context.is_frame_focused) {
    context.UnFocus();
    return;
  }

  auto frame_context_count{context.frame_contexts->GetSize()};

  if (frame_context_count == 0) {
    return;
  }

  context.Focus(frame_context_count - 1);
}
}  // namespace rendering
}  // namespace comet
#endif  // COMET_IMGUI
#endif  // COMET_PROFILING
