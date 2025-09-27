#include <iostream>
#include <string>
#include "simulation_params.hpp"
#include "laser.hpp"

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
    
    Common params;
    LaserData laser_data;
    
    std::cout << "Configuration..." << std::endl;
    load_cfg(params, laser_data);
    std::cout << "Configuration is done" << std::endl;

    LaserDevice ld;
    Pulse pl = ld.gen_pulse(laser_data.avg_count_photons);
    std::cout << "1: " << pl.count_photons << std::endl;
    std::cout << "2: " << pl.duration << std::endl;

    std::cout << "End" << std::endl;
    return 0;
}