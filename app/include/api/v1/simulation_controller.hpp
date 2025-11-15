#pragma once
#include <thread>
#include <atomic>
#include <mutex>
#include "simulation_params.hpp"
#include "bb84.hpp"
#include "test.hpp"

class SimulationController {
public:
    void load_config(const Common& params, const LaserData& laser, const QuantumChannelData& q_channel_data, const PhotodetectorData& ph_data);
    void start();
    void stop();
    bool is_running() const;
    bool stop_requested() const;

private:
    void simulation_thread_func();

    TestBB84 test_bb84;

    Common params_;
    LaserData laser_data_;
    QuantumChannelData channel_data_;
    PhotodetectorData ph_data_;
    std::thread worker_;
    mutable std::mutex mutex_;
    std::atomic<bool> running_{false};
    std::atomic<bool> stop_flag_{false};
};

extern SimulationController controller;