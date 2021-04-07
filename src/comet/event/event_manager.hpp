// Copyright 2021 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_CORE_EVENT_MANAGER_HPP_
#define COMET_CORE_EVENT_MANAGER_HPP_

#include "comet/event/event.hpp"
#include "comet/utils/structure/ring_queue.hpp"
#include "comet_precompile.hpp"

namespace comet {
class Engine;

namespace event {
class CallbackBase {
 public:
  virtual void Call(Event* event) = 0;
};

template <typename T, typename F>
class Callback : public CallbackBase {
 public:
  Callback(const F& function) : function_(function) {}

  virtual void Call(Event* event) override {
    function_(dynamic_cast<T&>(*event));
  }

 private:
  const F& function_;
};

class EventManager {
 public:
  EventManager(std::size_t = 200);

  template <typename T, typename F>
  bool Register(const F& function) {
    if (!std::is_base_of<Event, T>::value) {
      return false;
    }

    std::scoped_lock<std::mutex> lock(mutex_);
    auto& listeners = listeners_[static_cast<int>(T::kStaticType_)];
    listeners.emplace_back(
        std::move(std::make_unique<Callback<T, F>>(function)));

    return true;
  }

  template <typename T, typename... Targs>
  void FireEventNow(Targs... args) {
    Dispatch(Event::Create<T>(args...));
  }

  template <typename T, typename... Targs>
  void FireEvent(Targs... args) {
    std::scoped_lock<std::mutex> lock(mutex_);
    event_queue_.push(Event::Create<T>(args...));
  }

 private:
  friend Engine;
  mutable std::mutex mutex_;
  std::unordered_map<int, std::vector<std::unique_ptr<CallbackBase>>>
      listeners_;
  comet::structure::ring_queue<std::unique_ptr<Event>> event_queue_;

  void Dispatch(std::unique_ptr<Event>);
  void FireAllEvents();
};
}  // namespace event
}  // namespace comet

#endif  // COMET_CORE_EVENT_MANAGER_HPP_
