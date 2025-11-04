#include "api/v1/simulation_controller.hpp"
#include <iostream>

void SimulationController::load_config(const Common& params, const LaserData& laser) {
    std::lock_guard lock(mutex_);
    params_ = params;
    laser_data_ = laser;
}

void SimulationController::start() {
    if (running_) return;
    stop_flag_ = false;
    worker_ = std::thread(&SimulationController::simulation_thread_func, this);
    worker_.detach();
}

void SimulationController::stop() {
    stop_flag_ = true;
}

bool SimulationController::is_running() const { return running_.load(); }
bool SimulationController::stop_requested() const { return stop_flag_.load(); }

void SimulationController::simulation_thread_func() {
    running_ = true;
    switch (params_.protocol)
    {
    case 1: run_bb84(params_, laser_data_); break;
    // case 2: /* code */ break;
    default:
        break;
    }

    running_ = false;
}
