// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_EVENT_EVENT_MANAGER_H_
#define COMET_COMET_EVENT_EVENT_MANAGER_H_

#include <functional>

#include "comet/core/conf/configuration_manager.h"
#include "comet/core/essentials.h"
#include "comet/core/manager.h"
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

using EventListeners =
    std::unordered_map<stringid::StringId, std::vector<EventListener>>;
using IdEventTypeMap = std::unordered_map<EventListenerId, stringid::StringId>;
using EventQueue = LockFreeMPSCRingQueue<EventPointer>;

class EventManager : public Manager {
 public:
  static EventManager& Get();

  EventManager() = default;
  EventManager(const EventManager&) = delete;
  EventManager(EventManager&&) = delete;
  EventManager& operator=(const EventManager&) = delete;
  EventManager& operator=(EventManager&&) = delete;
  virtual ~EventManager() = default;

  void Shutdown() override;

  EventListenerId Register(const Callback& function,
                           stringid::StringId event_type);
  void Unregister(EventListenerId id);

  template <typename T, typename... Args>
  void FireEventNow(Args&&... args) const {
    Dispatch(GenerateEvent<T>(std::forward<Args>(args)...));
  }

  void FireEventNow(EventPointer event) const;

  template <typename T, typename... Args>
  void FireEvent(Args&&... args) {
    event_queue_.Push(GenerateEvent<T>(std::forward<Args>(args)...));
  }

  void FireEvent(EventPointer event);
  void FireAllEvents();

 private:
  void Dispatch(EventPointer event) const;

  EventListenerId listener_id_counter_{0};
  EventListeners listeners_{};
  IdEventTypeMap id_event_type_map_{};
  EventQueue event_queue_{COMET_CONF_U16(conf::kEventMaxQueueSize)};
};
}  // namespace event
}  // namespace comet

#endif  // COMET_COMET_EVENT_EVENT_MANAGER_H_
