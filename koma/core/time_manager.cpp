#include <chrono>
#include <iostream>

#include "time_manager.hpp"

double koma::TimeManager::GetNow() {
    auto current_time_ = std::chrono::system_clock::now();

    auto duration_in_seconds = std::chrono::duration<double>(
        current_time_.time_since_epoch()
    );

    return duration_in_seconds.count() * 1000;
}

void koma::TimeManager::Initialize() {
    this->previous_time_ = koma::TimeManager::GetNow();
}

void koma::TimeManager::Update() {
    this->current_time_ = koma::TimeManager::GetNow();
    this->time_delta_ = this->current_time_ - this->previous_time_;
    this->previous_time_ = this->current_time_;

    this->time_counter_ += this->time_delta_;

    if (this->time_counter_ > 1000) {
        this->notify_observers("second_elapsed");
        this->time_counter_ = 0.0;
    }
}

const double koma::TimeManager::GetTimeDelta() const {
    return this->time_delta_;
}

const double koma::TimeManager::GetCurrentTime() const {
    return this->current_time_;
}