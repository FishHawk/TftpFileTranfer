#ifndef SPEED_MONITOR_HPP
#define SPEED_MONITOR_HPP

#include <chrono>

class SpeedMonitor {
public:
    SpeedMonitor() {
        reset();
    }

    void reset() {
        timer_ = std::chrono::high_resolution_clock::now();
        interval_ = 0;
    }

    void tick() {
        auto now = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(now - timer_).count();
        timer_ = now;

        if (interval_ == 0)
            interval_ = duration;
        interval_ = smoothing_factor_ * interval_ + (1 - smoothing_factor_) * duration;
    }

    double speed() {
        return 1000 * 1000 / interval_;
    }

private:
    std::chrono::high_resolution_clock::time_point timer_;
    double interval_;
    double smoothing_factor_ = 0.9;
};

#endif
