// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_CORE_THREAD_THREAD_PROVIDER_H_
#define COMET_CORE_THREAD_THREAD_PROVIDER_H_

// External. ///////////////////////////////////////////////////////////////////
#include <utility>
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/job/worker_context.h"
#include "comet/core/essentials.h"
#include "comet/core/memory/allocator/allocator.h"
#include "comet/core/memory/default_allocator.h"
#include "comet/core/container/array.h"
#include "comet/core/container/iterator.h"

namespace comet {
namespace thread {
template <typename T>
class ThreadProvider {
 public:
  COMET_POPULATE_ITERATOR(T, this->array_.GetData(), this->array_.GetSize())

  virtual ~ThreadProvider() {
    COMET_ASSERT(!is_initialized_, "thread::ThreadProvider::~ThreadProvider",
                 "provider is still initialized");
  }

  void Initialize() {
    COMET_ASSERT(!is_initialized_, "thread::ThreadProvider::Initialize",
                 "provider is already initialized");
    OnInitialize();
    is_initialized_ = true;
  }

  void Destroy() {
    COMET_ASSERT(is_initialized_, "thread::ThreadProvider::Destroy",
                 "provider is not initialized");
    OnDestroy();
    array_.Release();
    is_initialized_ = false;
  }

  virtual T& Get() = 0;
  T& GetFromIndex(usize index) { return this->array_[index]; }

  ThreadId GetThreadIdFromIndex(usize index) {
    return static_cast<ThreadId>(index);
  }

  usize GetSize() const noexcept { return this->array_.GetSize(); }
  bool IsInitialized() const noexcept { return this->is_initialized_; }

 protected:
  ThreadProvider() = default;

  ThreadProvider(const ThreadProvider& other)
      : is_initialized_{other.is_initialized_}, array_{other.array_} {}

  ThreadProvider(ThreadProvider&& other) noexcept
      : is_initialized_{other.is_initialized_},
        array_{std::move(other.array_)} {
    other.is_initialized_ = false;
  }

  ThreadProvider& operator=(const ThreadProvider& other) {
    if (this == &other) {
      return *this;
    }

    is_initialized_ = other.is_initialized_;
    array_ = other.array_;
    return *this;
  }

  ThreadProvider& operator=(ThreadProvider&& other) noexcept {
    if (this == &other) {
      return *this;
    }

    is_initialized_ = other.is_initialized_;
    array_ = std::move(other.array_);

    other.is_initialized_ = false;
    return *this;
  }

  Array<T> array_{};

  virtual void OnInitialize() {}
  virtual void OnDestroy() {}

 private:
  bool is_initialized_{false};
};

template <typename T>
class FiberThreadProvider : public ThreadProvider<T> {
 public:
  FiberThreadProvider() = default;

  FiberThreadProvider(memory::Allocator* allocator) : allocator_{allocator} {
    COMET_ASSERT(allocator_ != nullptr,
                 "thread::FiberThreadProvider::FiberThreadProvider",
                 "allocator is null");
  }

  FiberThreadProvider(const FiberThreadProvider& other)
      : ThreadProvider<T>{other}, allocator_{other.allocator_} {}

  FiberThreadProvider(FiberThreadProvider&& other) noexcept
      : ThreadProvider<T>{std::move(other)}, allocator_{other.allocator_} {
    other.allocator_ = nullptr;
  }

  FiberThreadProvider& operator=(const FiberThreadProvider& other) {
    if (this == &other) {
      return *this;
    }

    ThreadProvider<T>::operator=(other);
    allocator_ = other.allocator_;
    return *this;
  }

  FiberThreadProvider& operator=(FiberThreadProvider&& other) noexcept {
    if (this == &other) {
      return *this;
    }

    ThreadProvider<T>::operator=(std::move(other));
    allocator_ = other.allocator_;

    other.allocator_ = nullptr;
    return *this;
  }

  ~FiberThreadProvider() override = default;

  T& Get() override {
    COMET_ASSERT(job::GetWorkerTag() == job::FiberWorker::kTag_,
                 "thread::FiberThreadProvider::Get",
                 "current worker is not a fiber worker");

    const auto type_index{job::GetWorkerTypeIndex()};

    COMET_ASSERT(type_index != job::kInvalidWorkerTypeIndex,
                 "thread::FiberThreadProvider::Get",
                 "worker type index is invalid");

    return this->array_[type_index];
  }

 protected:
  void OnInitialize() override {
    auto* allocator{allocator_ != nullptr ? allocator_
                                          : &memory::GetDefaultAllocator()};
    this->array_ = Array<T>{allocator};
    this->array_.Resize(job::GetCurrentFiberWorkerCount());
  }

 private:
  memory::Allocator* allocator_{nullptr};
};

template <typename T>
class IOThreadProvider : public ThreadProvider<T> {
 public:
  IOThreadProvider() = default;

  IOThreadProvider(memory::Allocator* allocator) : allocator_{allocator} {
    COMET_ASSERT(allocator_ != nullptr,
                 "thread::IOThreadProvider::IOThreadProvider",
                 "allocator is null");
  }

  IOThreadProvider(const IOThreadProvider& other)
      : ThreadProvider<T>{other}, allocator_{other.allocator_} {}

  IOThreadProvider(IOThreadProvider&& other) noexcept
      : ThreadProvider<T>{std::move(other)}, allocator_{other.allocator_} {
    other.allocator_ = nullptr;
  }

  IOThreadProvider& operator=(const IOThreadProvider& other) {
    if (this == &other) {
      return *this;
    }

    ThreadProvider<T>::operator=(other);
    allocator_ = other.allocator_;
    return *this;
  }

  IOThreadProvider& operator=(IOThreadProvider&& other) noexcept {
    if (this == &other) {
      return *this;
    }

    ThreadProvider<T>::operator=(std::move(other));
    allocator_ = other.allocator_;

    other.allocator_ = nullptr;
    return *this;
  }

  ~IOThreadProvider() override = default;

  T& Get() override {
    COMET_ASSERT(job::GetWorkerTag() == job::IOWorker::kTag_,
                 "thread::IOThreadProvider::Get",
                 "current worker is not an io worker");

    const auto type_index{job::GetWorkerTypeIndex()};

    COMET_ASSERT(type_index != job::kInvalidWorkerTypeIndex,
                 "thread::IOThreadProvider::Get",
                 "worker type index is invalid");

    return this->array_[type_index];
  }

 protected:
  void OnInitialize() override {
    auto* allocator{allocator_ != nullptr ? allocator_
                                          : &memory::GetDefaultAllocator()};
    this->array_ = Array<T>{allocator};
    this->array_.Resize(job::GetCurrentIOWorkerCount());
  }

 private:
  memory::Allocator* allocator_{nullptr};
};
}  // namespace thread
}  // namespace comet

#endif  // COMET_CORE_THREAD_THREAD_PROVIDER_H_