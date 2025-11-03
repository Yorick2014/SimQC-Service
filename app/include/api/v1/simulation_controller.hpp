#pragma once
#include <thread>
#include <atomic>
#include <mutex>
#include "simulation_params.hpp"
#include "bb84.hpp"

class SimulationController {
public:
    void load_config(const Common& params, const LaserData& laser);
    void start();
    void stop();
    bool is_running() const;
    bool stop_requested() const;

private:
    void simulation_thread_func();

    Common params_;
    LaserData laser_data_;
    std::thread worker_;
    std::atomic<bool> running_{false};
    std::atomic<bool> stop_flag_{false};
    std::mutex mutex_;
};
