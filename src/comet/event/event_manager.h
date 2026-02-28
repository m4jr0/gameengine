// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_EVENT_EVENT_MANAGER_H_
#define COMET_COMET_EVENT_EVENT_MANAGER_H_

// External. ///////////////////////////////////////////////////////////////////
#include <functional>
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/conf/configuration_manager.h"
#include "comet/core/essentials.h"
#include "comet/core/manager.h"
#include "comet/core/memory/allocator/free_list_allocator.h"
#include "comet/core/memory/allocator/platform_allocator.h"
#include "comet/core/type/array.h"
#include "comet/core/type/map.h"
#include "comet/core/type/ring_queue.h"
#include "comet/event/event.h"

namespace comet {
namespace event {
using EventListenerId = u32;
constexpr auto kInvalidEventListenerId{static_cast<EventListenerId>(-1)};
using Callback = std::function<void(Event&)>;

struct EventListener {
  EventListenerId id{kInvalidEventListenerId};
  Callback callback{};
};

using EventListeners = Map<stringid::StringId, Array<EventListener>>;
using IdEventTypeMap = Map<EventListenerId, stringid::StringId>;
using EventQueue = LockFreeMPSCRingQueue<EventPtr>;

class EventManager : public Manager {
 public:
  static EventManager& Get();

  EventManager();
  EventManager(const EventManager&) = delete;
  EventManager(EventManager&&) = delete;
  EventManager& operator=(const EventManager&) = delete;
  EventManager& operator=(EventManager&&) = delete;
  virtual ~EventManager() = default;

  void Initialize() override;
  void Shutdown() override;

  EventListenerId Register(const Callback& function,
                           stringid::StringId event_type);
  void Unregister(EventListenerId id);

  template <typename T, typename... Args>
  void FireEventNow(Args&&... args) const {
    Dispatch(GenerateEvent<T>(std::forward<Args>(args)...));
  }

  void FireEventNow(EventPtr event) const;

  template <typename T, typename... Args>
  void FireEvent(Args&&... args) {
    Add(GenerateEvent<T>(std::forward<Args>(args)...));
  }

  void FireEvent(EventPtr event);
  void FireAllEvents();

 private:
  void Add(EventPtr event);
  void Dispatch(EventPtr event) const;

  usize max_event_count_{COMET_CONF_U16(conf::kEventMaxQueueSize)};
  EventListenerId listener_id_counter_{0};
  EventListeners listeners_{};
  IdEventTypeMap id_event_type_map_{};
  EventQueue event_queue_{};
  memory::FiberFreeListAllocator listener_allocator_;

  // Platform allocator is used because it is only allocated once, during engine
  // startup.
  memory::PlatformAllocator event_queue_allocator_{
      memory::kEngineMemoryTagEvent};
};
}  // namespace event
}  // namespace comet

#endif  // COMET_COMET_EVENT_EVENT_MANAGER_H_
