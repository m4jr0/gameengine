// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "environment_debug_ui.h"
////////////////////////////////////////////////////////////////////////////////

#ifdef COMET_HAS_DEBUG_UI

// External. ///////////////////////////////////////////////////////////////////
#include "imgui.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/math/math_scalar.h"

namespace comet {
namespace debugui {
namespace internal {
static void DrawTimeLabel(f32 hours) {
  const auto wrapped{math::Wrap(hours, 24.0f)};
  const auto h{static_cast<s32>(wrapped)};
  const auto m{static_cast<s32>((wrapped - static_cast<f32>(h)) * 60.0f)};
  ImGui::Text("Current Time: %02d:%02d", h, m);
}
}  // namespace internal

void EnvironmentDebugUi::Draw(
    environment::EnvironmentManager& environment) const {
  ImGui::Begin("Environment");
  ImGui::Indent();

  if (ImGui::CollapsingHeader("Time", ImGuiTreeNodeFlags_DefaultOpen)) {
    auto is_frozen{environment.IsTimeFrozen()};

    if (ImGui::Checkbox("Freeze Time", &is_frozen)) {
      environment.SetTimeFrozen(is_frozen);
    }

    auto time_of_day{environment.GetTimeOfDayHours()};

    if (ImGui::SliderFloat("Time of Day", &time_of_day, .0f, 24.0f, "%.2f h")) {
      environment.SetTimeOfDayHours(time_of_day);
    }

    internal::DrawTimeLabel(environment.GetTimeOfDayHours());

    if (ImGui::Button("Sunrise")) {
      environment.SetTimeOfDayHours(6.0f);
    }

    ImGui::SameLine();

    if (ImGui::Button("Noon")) {
      environment.SetTimeOfDayHours(12.0f);
    }

    ImGui::SameLine();

    if (ImGui::Button("Sunset")) {
      environment.SetTimeOfDayHours(18.0f);
    }

    ImGui::SameLine();

    if (ImGui::Button("Midnight")) {
      environment.SetTimeOfDayHours(.0f);
    }

    ImGui::Separator();

    auto effective_day_duration{environment.GetEffectiveDayDurationSeconds()};

    if (ImGui::DragFloat("Effective Day Duration (s)", &effective_day_duration,
                         .1f, .1f, 86400.0f, "%.2f s")) {
      environment.SetEffectiveDayDurationSeconds(effective_day_duration);
    }

    auto effective_acceleration{
        environment.GetEffectiveDayAccelerationFactor()};

    if (ImGui::DragFloat("Effective Acceleration Factor",
                         &effective_acceleration, .05f, .001f, 10000.0f,
                         "%.3fx")) {
      environment.SetEffectiveDayAccelerationFactor(effective_acceleration);
    }

    if (ImGui::TreeNode("Advanced Time Controls")) {
      auto day_time_scale{environment.GetDayTimeScale()};
      if (ImGui::DragFloat("Day Time Scale", &day_time_scale, .01f, .0f,
                           1000.0f, "%.3fx")) {
        environment.SetDayTimeScale(day_time_scale);
      }

      auto base_day_duration{environment.GetDayDurationSeconds()};

      if (ImGui::DragFloat("Base Day Duration (s)", &base_day_duration, .1f,
                           .1f, 86400.0f, "%.2f s")) {
        environment.SetDayDurationSeconds(base_day_duration);
      }

      ImGui::Text("Effective Day Duration: %.2f s",
                  environment.GetEffectiveDayDurationSeconds());
      ImGui::Text("Effective Acceleration: %.3fx",
                  environment.GetEffectiveDayAccelerationFactor());

      ImGui::TreePop();
    }

    ImGui::Separator();

    auto duration_h{environment.GetEffectiveDayDurationHoursPart()};
    auto duration_m{environment.GetEffectiveDayDurationMinutesPart()};
    auto duration_s{environment.GetEffectiveDayDurationSecondsPart()};

    auto changed{false};
    changed |= ImGui::InputInt("Day Duration Hours", &duration_h);
    changed |= ImGui::InputInt("Day Duration Minutes", &duration_m);
    changed |= ImGui::InputInt("Day Duration Seconds", &duration_s);

    if (changed) {
      environment.SetEffectiveDayDurationHms(duration_h, duration_m,
                                             duration_s);
    }

    if (ImGui::Button("10 sec/day")) {
      environment.SetEffectiveDayDurationSeconds(10.0f);
    }

    ImGui::SameLine();

    if (ImGui::Button("20 sec/day")) {
      environment.SetEffectiveDayDurationSeconds(20.0f);
    }

    ImGui::SameLine();

    if (ImGui::Button("60 sec/day")) {
      environment.SetEffectiveDayDurationSeconds(60.0f);
    }

    ImGui::SameLine();

    if (ImGui::Button("5 min/day")) {
      environment.SetEffectiveDayDurationSeconds(60.0f * 5.0f);
    }

    ImGui::SameLine();

    if (ImGui::Button("10 min/day")) {
      environment.SetEffectiveDayDurationSeconds(60.0f * 10.0f);
    }

    ImGui::SameLine();

    if (ImGui::Button("25 min/day")) {
      environment.SetEffectiveDayDurationSeconds(60.0f * 25.0f);
    }
  }

  if (ImGui::CollapsingHeader("Playback Window",
                              ImGuiTreeNodeFlags_DefaultOpen)) {
    auto use_day_window{environment.IsUsingDayWindow()};

    if (ImGui::Checkbox("Use Day Window", &use_day_window)) {
      environment.SetUseDayWindow(use_day_window);
    }

    auto window_start{environment.GetDayWindowStartHours()};
    auto window_end{environment.GetDayWindowEndHours()};

    if (ImGui::SliderFloat("Window Start", &window_start, .0f, 24.0f,
                           "%.2f h")) {
      environment.SetDayWindowStartHours(window_start);
    }

    if (ImGui::SliderFloat("Window End", &window_end, .0f, 24.0f, "%.2f h")) {
      environment.SetDayWindowEndHours(window_end);
    }

    ImGui::Text("Effective Time: %.2f h",
                environment.GetEffectiveTimeOfDayHours());
  }

  if (ImGui::CollapsingHeader("Sun / Sky", ImGuiTreeNodeFlags_DefaultOpen)) {
    const auto azimuth_offset_rad{environment.GetAzimuthOffsetRadians()};
    auto azimuth_offset_deg{math::ConvertToDegrees(azimuth_offset_rad)};

    if (ImGui::SliderFloat("Azimuth Offset", &azimuth_offset_deg, -180.0f,
                           180.0f, "%.1f deg")) {
      environment.SetAzimuthOffsetRadians(
          math::ConvertToRadians(azimuth_offset_deg));
    }

    auto ambient{environment.ComputeAmbientColor()};
    ImGui::ColorEdit3("Ambient Color", &ambient.x,
                      ImGuiColorEditFlags_DisplayRGB |
                          ImGuiColorEditFlags_NoInputs |
                          ImGuiColorEditFlags_NoPicker);
  }

  ImGui::Unindent();
  ImGui::End();
}
}  // namespace debugui
}  // namespace comet

#endif  // COMET_HAS_DEBUG_UI