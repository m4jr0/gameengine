// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_EVENT_EVENT_MANAGER_H_
#define COMET_COMET_EVENT_EVENT_MANAGER_H_

#include "comet_precompile.h"

#include "comet/event/event.h"
#include "comet/utils/structure/ring_queue.h"

namespace comet {
class Engine;

namespace event {
using EventListener = std::function<void(Event&)>;

class EventManager {
 public:
  EventManager(uindex queue_size = 200);
  EventManager(const EventManager&) = delete;
  EventManager(EventManager&&) = delete;
  EventManager& operator=(const EventManager&) = delete;
  EventManager& operator=(EventManager&&) = delete;
  ~EventManager() = default;

  void Register(const EventListener& function, stringid::StringId event_type);

  template <typename T, typename... Targs>
  void FireEventNow(Targs... args) {
    Dispatch(Event::Create<T>(args...));
  }

  void FireEventNow(std::unique_ptr<Event> event);

  template <typename T, typename... Targs>
  void FireEvent(Targs... args) {
    std::scoped_lock<std::mutex> lock(mutex_);
    event_queue_.push(Event::Create<T>(args...));
  }

  void FireEvent(std::unique_ptr<Event> event);

 private:
  friend Engine;
  mutable std::mutex mutex_;
  std::unordered_map<stringid::StringId, std::vector<EventListener>> listeners_;
  comet::utils::structure::ring_queue<std::unique_ptr<event::Event>>
      event_queue_;

  void Dispatch(std::unique_ptr<Event>);
  void FireAllEvents();
};
}  // namespace event
}  // namespace comet

#endif  // COMET_COMET_EVENT_EVENT_MANAGER_H_
