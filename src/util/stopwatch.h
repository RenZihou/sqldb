// stopwatch.h
// -*- encoding: utf-8 -*-
// @Author: RenZihou

#ifndef STOPWATCH_H_
#define STOPWATCH_H_

#include <chrono>

class Stopwatch {
private:
    std::chrono::time_point<std::chrono::steady_clock> start_time;
    std::chrono::time_point<std::chrono::steady_clock> end_time;
    bool running = false;

public:
    Stopwatch() = default;

    void start() {
        this->start_time = std::chrono::steady_clock::now();
        this->running = true;
    }

    void stop() {
        this->end_time = std::chrono::steady_clock::now();
        this->running = false;
    }

    [[nodiscard]] long long getElapsedMilliseconds() const {
        if (this->running) {
            return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - this->start_time).count();
        } else {
            return std::chrono::duration_cast<std::chrono::milliseconds>(this->end_time - this->start_time).count();
        }
    }

    [[nodiscard]] double getElapsedSeconds() const {
        return static_cast<double>(this->getElapsedMilliseconds()) * 1e-3;
    }
};

#endif  // STOPWATCH_H_
