#include "api/v1/simulation_controller.hpp"
#include <iostream>

SimulationController controller;

void SimulationController::load_config(const Common& params, const LaserData& laser, const QuantumChannelData& q_channel_data, const PhotodetectorData& ph_data) {
    std::cout << "load_config() called" << std::endl;
    std::lock_guard lock(mutex_);
    params_ = params;
    laser_data_ = laser;
    channel_data_ = q_channel_data;
    ph_data_ = ph_data;
    
    std::cout << "load_config() end" << std::endl;
}

void SimulationController::start() {
    std::cout << "start() called" << std::endl;
    if (running_) return;
    stop_flag_ = false;
    worker_ = std::thread(&SimulationController::simulation_thread_func, this);
    std::cout << "thread created" << std::endl;
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
    case 1: std::cout << "BB84" << std::endl; test_bb84.run(params_, laser_data_, channel_data_, ph_data_); break;
    default:
        std::cout << "default case" << std::endl;
        break;
    }

    running_ = false;
}
