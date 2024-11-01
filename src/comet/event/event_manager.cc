// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "event_manager.h"

namespace comet {
namespace event {
EventManager& EventManager::Get() {
  static EventManager singleton{};
  return singleton;
}

void EventManager::Initialize() {
  Manager::Initialize();
  COMET_ASSERT(max_event_count_ > 0,
               "Max event count is invalid: ", max_event_count_, ".");
  event_queue_allocator_.Initialize();
  event_queue_ = EventQueue{&event_queue_allocator_, max_event_count_};
}

void EventManager::Shutdown() {
  listeners_.clear();
  event_queue_.Clear();
  event_queue_allocator_.Destroy();
  Manager::Shutdown();
}

EventListenerId EventManager::Register(const Callback& function,
                                       stringid::StringId event_type) {
  auto& listeners{listeners_[event_type]};
  const auto id{listener_id_counter_++};
  listeners.push_back({id, function});
  id_event_type_map_[id] = event_type;
  return id;
}

void EventManager::Unregister(EventListenerId id) {
  COMET_ASSERT(id_event_type_map_.find(id) != id_event_type_map_.cend(),
               "Unable to find event type from ID ", id, "!");
  auto& listeners{listeners_[id_event_type_map_.at(id)]};
  usize found_index{kInvalidIndex};

  for (usize i{0}; i < listeners.size(); ++i) {
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

void EventManager::FireEventNow(EventPointer event) const {
  Dispatch(std::move(event));
}

void EventManager::FireEvent(EventPointer event) { Add(std::move(event)); }

void EventManager::FireAllEvents() {
  EventPointer event;

  while (event_queue_.TryPop(event)) {
    Dispatch(std::move(event));
  }

  event_count_ = 0;
}

void EventManager::Add(EventPointer event) {
  if (event_count_ == max_event_count_ - 1) {
    COMET_LOG_EVENT_WARNING("Max event reached: ", max_event_count_,
                            "! Ignoring event.");
    return;
  }

  event_queue_.Push(std::move(event));
  ++event_count_;
}

void EventManager::Dispatch(EventPointer event) const {
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