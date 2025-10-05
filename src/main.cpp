#include <iostream>
#include <string>
#include <time.h>
#include <complex>
#include <cmath>
#include <vector>
#include "simulation_params.hpp"
#include "laser.hpp"
#include "sequence_generator.hpp"

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
    LaserData l_d;

    std::cout << "Configuration..." << std::endl;
    load_cfg(params, l_d);
    std::cout << "Configuration is done" << std::endl;

    clock_t time = clock();
    std::cout << "Time: " << (double)(time - tStart) / CLOCKS_PER_SEC << " sec" << std::endl;

    // --- laser operation ---
    AttLaser laser(l_d.central_wavelength, l_d.laser_power_w, l_d.attenuation_db, l_d.pulse_duration, l_d.repeat_rate);
    // for (int i = 0; i < 5; ++i) {
    //     Pulse p = laser.generate_pulse();
    //     std::cout << "Pulse " << i
    //               << " | Photons: " << p.count_photons
    //               << " | Timestamp: " << p.timestamp << " s\n";
    // }
    
    std::vector<Pulse> seq_pulses;
    for (uint16_t i = 0; i < 10; i++)
    {
        Pulse pl = laser.generate_pulse();
        seq_pulses.push_back(pl);
    }
    Pulse pl1 = seq_pulses[2];
    std::cout << pl1.timestamp << std::endl;

    //////////////////////////////////////////
    ///             modulator              ///

    SequenceGenerator gen(42);
    gen.generate(10);
    const auto& seq = gen.get_sequence();

    for (const auto& qubit : seq) {
        std::cout << "Bit: " << static_cast<int>(qubit.bit)
                << ", Basis: " << (qubit.basis == Basis::rectilinear ? "R" : "D")
                << "\n";
    }
    std::cout << "четвёртый кубит" << std::endl;

    const Qubit& q = gen[3];
    std::cout << "Bit: " << static_cast<int>(q.bit)
            << ", Basis: " << (q.basis == Basis::rectilinear ? "R" : "D") 
            << "\n";

    ///             modulator              ///
    //////////////////////////////////////////


    time = clock();
    std::cout << "Time: " << (double)(time - tStart) / CLOCKS_PER_SEC << " sec" << std::endl;

    // JonesVector v = getJonesVector(Polarization::RCP);

    std::cout << "End" << std::endl;
    return 0;
}