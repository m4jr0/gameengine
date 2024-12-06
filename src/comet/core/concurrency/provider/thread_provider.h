// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_CONCURRENCY_PROVIDER_THREAD_PROVIDER_H_
#define COMET_COMET_CORE_CONCURRENCY_PROVIDER_THREAD_PROVIDER_H_

#include "comet/core/concurrency/job/worker_context.h"
#include "comet/core/essentials.h"
#include "comet/core/memory/allocator/allocator.h"
#include "comet/core/type/array.h"

namespace comet {
namespace thread {
template <typename T>
class ThreadProvider {
 public:
  virtual ~ThreadProvider() {
    COMET_ASSERT(!is_initialized_,
                 "Destructor called for thread provider, but it is still "
                 "initialized!");
  }

  virtual void Initialize() {
    COMET_ASSERT(
        !is_initialized_,
        "Tried to initialize thread provider, but it is already done!");
    is_initialized_ = true;
  }

  virtual void Destroy() {
    COMET_ASSERT(
        is_initialized_,
        "Tried to destroy thread provider, but it is not initialized!");
    array_.Destroy();
    is_initialized_ = false;
  }

  virtual T& Get() = 0;
  T& GetFromIndex(usize index) { return this->array_[index]; }
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

  bool is_initialized_ = false;
  FixedArray<T> array_{};
};  // namespace thread

template <typename T>
class FiberThreadProvider : public ThreadProvider<T> {
 public:
  FiberThreadProvider() = default;

  FiberThreadProvider(memory::Allocator* allocator) : allocator_{allocator} {
    COMET_ASSERT(allocator_ != nullptr, "Allocator is null!");
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

  void Initialize() override {
    ThreadProvider<T>::Initialize();
    this->array_ = FixedArray<T>{allocator_, job::GetCurrentFiberWorkerCount()};
  }

  T& Get() override {
    COMET_ASSERT(job::GetWorkerTag() == job::FiberWorker::kTag_,
                 "Tried to provide outside an I/O worker!");
    auto type_index{job::GetWorkerTypeIndex()};
    COMET_ASSERT(type_index != job::kInvalidWorkerTypeIndex,
                 "Invalid worker type index retrieved!");
    return this->array_[type_index];
  }

 private:
  memory::Allocator* allocator_{nullptr};
};

template <typename T>
class IOThreadProvider : public ThreadProvider<T> {
 public:
  IOThreadProvider() = default;

  IOThreadProvider(memory::Allocator* allocator) : allocator_{allocator} {
    COMET_ASSERT(allocator_ != nullptr, "Allocator is null!");
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

  void Initialize() override {
    ThreadProvider<T>::Initialize();
    this->array_ = FixedArray<T>{allocator_, job::GetCurrentIOWorkerCount()};
  }

  T& Get() override {
    COMET_ASSERT(job::GetWorkerTag() == job::IOWorker::kTag_,
                 "Tried to provide outside an I/O worker!");
    auto type_index{job::GetWorkerTypeIndex()};
    COMET_ASSERT(type_index != job::kInvalidWorkerTypeIndex,
                 "Invalid worker type index retrieved!");
    return this->array_[type_index];
  }

 private:
  memory::Allocator* allocator_{nullptr};
};
}  // namespace thread
}  // namespace comet

#endif  // COMET_COMET_CORE_CONCURRENCY_PROVIDER_THREAD_PROVIDER_H_