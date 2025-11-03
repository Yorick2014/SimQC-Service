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
    try {
        std::unique_ptr<ILaser> laser = LaserFactory::create(params_.laser_type, laser_data_);
        Alice alice(*laser, params_.seed_Alice);
        Bob bob(params_.seed_Bob);

        auto N = params_.num_pulses;
        auto sent_pulses = alice.generate_pulses(N);

        for (size_t i = 0; i < N && !stop_flag_; ++i) {
            // здесь имитация задержки и вычислений
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }

        std::cout << "[Simulation finished]" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Simulation error: " << e.what() << std::endl;
    }

    running_ = false;
}
