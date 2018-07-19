#ifndef TIME_MANAGER_HPP
#define TIME_MANAGER_HPP

#include <chrono>

#include "../utils/subject.hpp"

namespace koma {
class TimeManager : public koma::Subject {
public:
    virtual ~TimeManager() = default;
    static double GetNow();
    void Initialize();
    void Update();
    const double GetTimeDelta() const;
    const double GetCurrentTime() const;

private:
    double current_time_ = 0.0;
    double previous_time_ = 0.0;
    double time_delta_ = 0.0;
    double time_counter_ = 0.0;
};
}; // namespace koma

#endif // TIME_MANAGER_HPP
