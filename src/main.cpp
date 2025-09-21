#include <iostream>
#include <string>
#include "simulation_params.hpp"


int main() {
    std::cout << "Start" << std::endl;
    
    Common params;
    try {
        params = load_config<Common>("/app/cfg/common.json");
    } 
    catch (const std::exception& e) {
        std::cerr << "Ошибка конфигурации: " << e.what() << '\n';
    }

    std::cout << "End" << std::endl;
    return 0;
}