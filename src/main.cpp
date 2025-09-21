#include <iostream>
#include <string>
#include "simulation_params.hpp"

void load_cfg(Common &params, Laser &laser){
    try {
        params = load_config<Common>("/app/cfg/common.json");
        laser = load_config<Laser>("/app/cfg/laser_params.json");
    } 
    catch (const std::exception& e) {
        std::cerr << "Ошибка конфигурации: " << e.what() << '\n';
    }

}

int main() {
    std::cout << "Start" << std::endl;
    
    Common params;
    Laser laser;
    std::cout << "Configuration..." << std::endl;
    load_cfg(params, laser);
    std::cout << "Configuration is done" << std::endl;

    std::cout << "End" << std::endl;
    return 0;
}