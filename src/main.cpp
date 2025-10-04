#include <iostream>
#include <string>
#include <time.h>
#include <complex>
#include <cmath>
#include <vector>
#include "simulation_params.hpp"
#include "laser.hpp"
#include "sequence_generator.hpp"
#include "bb84.hpp"

void load_cfg(Common &params, LaserData &laser_data){
    try {
        params = load_config<Common>("/app/cfg/common.json");
        laser_data = load_config<LaserData>("/app/cfg/laser_params.json");
    } 
    catch (const std::exception& e) {
        std::cerr << "Ошибка конфигурации: " << e.what() << '\n';
    }
}

int main() {
    std::cout << "Start" << std::endl;
    clock_t tStart = clock();
    
    Common params;
    LaserData laser_data;

    std::cout << "Configuration..." << std::endl;
    load_cfg(params, laser_data);
    std::cout << "Configuration is done" << std::endl;

    clock_t time = clock();
    std::cout << "Time: " << (double)(time - tStart) / CLOCKS_PER_SEC << " sec" << std::endl;

    // --- laser operation ---

    // pulse sequence generation
    // std::vector<Pulse> seq_pulses;
    // for (uint16_t i = 0; i < 10; i++)
    // {
    //     Pulse pl = ld.gen_pulse(laser_data.avg_count_photons, laser_data.pulse_duration);
    //     seq_pulses.push_back(pl);
    // }

    time = clock();
    std::cout << "Time: " << (double)(time - tStart) / CLOCKS_PER_SEC << " sec" << std::endl;

    // JonesVector v = getJonesVector(Polarization::RCP);

    std::cout << "End" << std::endl;
    return 0;
}