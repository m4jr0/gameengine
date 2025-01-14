// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "comet_pch.h"

#include "event_manager.h"

#include <utility>

#include "comet/core/memory/memory.h"
#include "comet/math/math_commons.h"

namespace comet {
namespace event {
EventManager& EventManager::Get() {
  static EventManager singleton{};
  return singleton;
}

EventManager::EventManager()
    : listener_allocator_{
          math::Max(
              sizeof(Pair<stringid::StringId, Array<EventListener>>),
              math::Max(sizeof(EventListener),
                        sizeof(Pair<EventListenerId, stringid::StringId>))),
          4096, memory::kEngineMemoryTagEvent} {}

void EventManager::Initialize() {
  Manager::Initialize();
  COMET_ASSERT(max_event_count_ > 0,
               "Max event count is invalid: ", max_event_count_, ".");
  listener_allocator_.Initialize();
  event_queue_allocator_.Initialize();

  listeners_ = EventListeners{&listener_allocator_};
  id_event_type_map_ = IdEventTypeMap{&listener_allocator_};
  event_queue_ = EventQueue{&event_queue_allocator_, max_event_count_};
}

void EventManager::Shutdown() {
  listeners_ = {};
  id_event_type_map_ = {};
  event_queue_ = {};
  event_queue_allocator_.Destroy();
  listener_allocator_.Destroy();
  Manager::Shutdown();
}

EventListenerId EventManager::Register(const Callback& function,
                                       stringid::StringId event_type) {
  auto& listeners{listeners_[event_type]};
  const auto id{listener_id_counter_++};
  listeners.EmplaceBack(id, function);
  id_event_type_map_[id] = event_type;
  return id;
}

void EventManager::Unregister(EventListenerId id) {
  COMET_ASSERT(id_event_type_map_.IsContained(id),
               "Unable to find event type from ID ", id, "!");
  auto& listeners{listeners_[id_event_type_map_.Get(id)]};
  usize found_index{kInvalidIndex};

  for (usize i{0}; i < listeners.GetSize(); ++i) {
    if (listeners[i].id == id) {
      found_index = i;
      break;
    }
  }

  COMET_ASSERT(found_index != kInvalidIndex, "Unable to find listener from ID ",
               id, "!");
  listeners.RemoveFromPos(listeners.begin() + found_index);
  id_event_type_map_.Remove(id);
}

void EventManager::FireEventNow(EventPtr event) const {
  Dispatch(std::move(event));
}

void EventManager::FireEvent(EventPtr event) { Add(std::move(event)); }

void EventManager::FireAllEvents() {
  EventPtr event;

  while (event_queue_.TryPop(event)) {
    Dispatch(std::move(event));
  }
}

void EventManager::Add(EventPtr event) { event_queue_.Push(std::move(event)); }

void EventManager::Dispatch(EventPtr event) const {
  auto* listeners{listeners_.TryGet(event->GetType())};

  if (listeners == nullptr) {
    return;
  }

  const auto event_pointer{event.get()};

  for (const auto& listener : *listeners) {
    listener.callback(*event_pointer);
  }
}
}  // namespace event
}  // namespace comet