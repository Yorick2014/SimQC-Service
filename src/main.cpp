#include <iostream>
#include <string>
#include <time.h>
#include <complex>
#include <cmath>
#include "simulation_params.hpp"
#include "laser.hpp"

using JonesVector = std::array<std::complex<double>, 2>;
JonesVector get_jones_vector(Polarization p);

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
    std::cout << "Время выполнения: " << (double)(time - tStart) / CLOCKS_PER_SEC << " секунд" << std::endl;
    LaserDevice ld;
    Pulse pl = ld.gen_pulse(laser_data.avg_count_photons);

    std::cout << "1: " << pl.count_photons << std::endl;
    std::cout << "2: " << pl.duration << std::endl;
    time = clock();
    std::cout << "Время выполнения: " << (double)(time - tStart) / CLOCKS_PER_SEC << " секунд" << std::endl;

    // JonesVector v = getJonesVector(Polarization::RCP);

    std::cout << "End" << std::endl;
    return 0;
}

JonesVector get_jones_vector(Polarization p) {
    static const double inv_sqrt2 = 1.0 / std::sqrt(2.0);
    using cd = std::complex<double>;
    switch (p) {
        case Polarization::horizontal:
            return { cd(1,0), cd(0,0) };
        case Polarization::vertical:
            return { cd(0,0), cd(1,0) };
        case Polarization::diagonal:
            return { cd(inv_sqrt2,0), cd(inv_sqrt2,0) };
        case Polarization::antidiagonal:
            return { cd(inv_sqrt2,0), cd(-inv_sqrt2,0) };
        case Polarization::RCP:
            return { cd(inv_sqrt2,0), cd(0,-inv_sqrt2) }; // [1, -i]/√2
        case Polarization::LCP:
            return { cd(inv_sqrt2,0), cd(0, inv_sqrt2) }; // [1, +i]/√2
    }
    return { cd(0,0), cd(0,0) }; // fallback
}