// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_EVENT_EVENT_MANAGER_H_
#define COMET_COMET_EVENT_EVENT_MANAGER_H_

#include "comet_precompile.h"

#include "comet/core/manager.h"
#include "comet/core/type/structure/ring_queue.h"
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
using EventQueue = comet::ring_queue<std::unique_ptr<event::Event>>;

struct EventManagerDescr : ManagerDescr {
  uindex queue_size{200};
};

class EventManager : public Manager {
 public:
  EventManager() = delete;
  explicit EventManager(const EventManagerDescr& descr);
  EventManager(const EventManager&) = delete;
  EventManager(EventManager&&) = delete;
  EventManager& operator=(const EventManager&) = delete;
  EventManager& operator=(EventManager&&) = delete;
  virtual ~EventManager() = default;

  void Shutdown() override;

  EventListenerId Register(const Callback& function,
                           stringid::StringId event_type);
  void Unregister(EventListenerId id);

  template <typename T, typename... Targs>
  void FireEventNow(Targs... args) const {
    Dispatch(Event::Generate<T>(args...));
  }

  void FireEventNow(std::unique_ptr<Event> event) const;

  template <typename T, typename... Targs>
  void FireEvent(Targs... args) {
    std::scoped_lock<std::mutex> lock(mutex_);
    event_queue_.push(Event::Generate<T>(args...));
  }

  void FireEvent(std::unique_ptr<Event> event);
  void FireAllEvents();

 private:
  void Dispatch(std::unique_ptr<Event> event) const;

  mutable std::mutex mutex_{};
  EventListenerId listener_id_counter_{0};
  EventListeners listeners_{};
  IdEventTypeMap id_event_type_map_{};
  EventQueue event_queue_{0};
};
}  // namespace event
}  // namespace comet

#endif  // COMET_COMET_EVENT_EVENT_MANAGER_H_
