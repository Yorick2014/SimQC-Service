#include <iostream>
#include <string>
#include "simulation_params.hpp"
// #include "pulse.hpp"


int main() {
    std::cout << "Start" << std::endl;
    
    Common params;

    // Laser laser;
    // QuantumChannel q_channel;
    try {
        params = load_config<Common>("/app/cfg/common.json");
        // laser = load_config<Laser>("/home/user/SimQC/cfg/laser_params.json"); // cfg/laser_params.json
        // q_channel = load_config<QuantumChannel>("/home/user/SimQC/cfg/channel_params.json");
    } catch (const std::exception& e) {
        std::cerr << "Ошибка конфигурации: " << e.what() << '\n';
    }
    
    std::cout << params.protocol << std::endl;

    std::cout << "End" << std::endl;
    return 0;
}