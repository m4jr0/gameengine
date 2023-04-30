// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DEBUGGER_DEBUGGUER_DISPLAYER_MANAGER_H_
#define COMET_COMET_RENDERING_DEBUGGER_DEBUGGUER_DISPLAYER_MANAGER_H_

#include "comet_precompile.h"

#include "comet/core/manager.h"
#include "comet/rendering/rendering_common.h"

#ifdef COMET_DEBUG
namespace comet {
namespace rendering {
struct MiniProfilerPacket {
  f32 physics_frame_time{0};
  u32 physics_frame_rate{0};
  f32 rendering_frame_time{0};
  u32 rendering_frame_rate{0};
  u32 rendering_draw_count{0};
  DriverType rendering_driver_type{DriverType::Unknown};
};

using DebuggerDisplayerManagerDescr = ManagerDescr;

class DebuggerDisplayerManager : public Manager {
 public:
  DebuggerDisplayerManager() = delete;
  explicit DebuggerDisplayerManager(const DebuggerDisplayerManagerDescr& descr);
  DebuggerDisplayerManager(const DebuggerDisplayerManager&) = delete;
  DebuggerDisplayerManager(DebuggerDisplayerManager&&) = delete;
  DebuggerDisplayerManager& operator=(const DebuggerDisplayerManager&) = delete;
  DebuggerDisplayerManager& operator=(DebuggerDisplayerManager&&) = delete;
  virtual ~DebuggerDisplayerManager() = default;

  void Update(const MiniProfilerPacket& packet);
  void Draw();

 private:
  MiniProfilerPacket mini_profiler_packet_{};
};
}  // namespace rendering
}  // namespace comet
#endif  // COMET_DEBUG

#endif  // COMET_COMET_RENDERING_DEBUGGER_DEBUGGUER_DISPLAYER_MANAGER_H_
