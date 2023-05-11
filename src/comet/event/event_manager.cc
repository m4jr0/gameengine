// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "event_manager.h"

namespace comet {
namespace event {
EventManager::EventManager(const EventManagerDescr& descr)
    : Manager{descr}, event_queue_{descr.queue_size} {}

void EventManager::Shutdown() {
  listeners_.clear();
  event_queue_.clear();
  Manager::Shutdown();
}

EventListenerId EventManager::Register(const Callback& function,
                                       stringid::StringId event_type) {
  std::scoped_lock<std::mutex> lock(mutex_);
  auto& listeners{listeners_[event_type]};
  const auto id{listener_id_counter_++};
  listeners.push_back({id, function});
  id_event_type_map_[id] = event_type;
  return id;
}

void EventManager::Unregister(EventListenerId id) {
  std::scoped_lock<std::mutex> lock(mutex_);
  COMET_ASSERT(id_event_type_map_.find(id) != id_event_type_map_.cend(),
               "Unable to find event type from ID ", id, "!");
  auto& listeners{listeners_[id_event_type_map_.at(id)]};
  uindex found_index{kInvalidIndex};

  for (uindex i{0}; i < listeners.size(); ++i) {
    if (listeners[i].id == id) {
      found_index = i;
      break;
    }
  }

  COMET_ASSERT(found_index != kInvalidIndex, "Unable to find listener from ID ",
               id, "!");
  listeners.erase(listeners.begin() + found_index);
  id_event_type_map_.erase(id);
}

void EventManager::FireEventNow(std::unique_ptr<Event> event) const {
  Dispatch(std::move(event));
}

void EventManager::FireEvent(std::unique_ptr<Event> event) {
  std::scoped_lock<std::mutex> lock(mutex_);
  event_queue_.push(std::move(event));
}

void EventManager::FireAllEvents() {
  std::scoped_lock<std::mutex> lock(mutex_);
  std::unique_ptr<Event> event;

  while (!event_queue_.empty()) {
    Dispatch(std::move(event_queue_.front()));
    event_queue_.pop();
  }
}

void EventManager::Dispatch(std::unique_ptr<Event> event) const {
  auto it{listeners_.find(event->GetType())};

  if (it == listeners_.end()) {
    return;
  }

  const auto& listeners{it->second};
  const auto event_pointer{event.get()};

  for (const auto& listener : listeners) {
    listener.callback(*event_pointer);
  }
}
}  // namespace event
}  // namespace comet