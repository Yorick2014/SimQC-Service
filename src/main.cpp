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

    // --- Шаг 1: загрузка конфигурации ---
    Common params;
    LaserData laser_data;
    load_cfg(params, laser_data);

    // --- Шаг 2: инициализация лазера ---
    AttLaser laser(
        laser_data.central_wavelength,
        laser_data.laser_power_w,
        laser_data.attenuation_db,
        laser_data.pulse_duration,
        laser_data.repeat_rate
    );

    // --- Шаг 3: создание Алисы и Боба ---
    Alice alice(laser, 42);
    Bob bob(1337);

    // --- Шаг 4: Алиса генерирует импульсы ---
    size_t N = 20;
    auto sent_pulses = alice.generate_pulses(N);

    std::cout << "\n[ALICE] Generated pulses:\n";
    std::cout << "Idx | Bit | Basis | Polarization | Photons | Timestamp (ns)\n";
    std::cout << "-----------------------------------------------------------\n";
    for (size_t i = 0; i < sent_pulses.size(); ++i) {
        const auto& sp = sent_pulses[i];
        std::string basis_str = (sp.qubit.basis == Basis::rectilinear) ? "R" : "D";
        std::string pol_str;
        switch (sp.pulse.polarization) {
            case Polarization::horizontal: pol_str = "H"; break;
            case Polarization::vertical: pol_str = "V"; break;
            case Polarization::diagonal: pol_str = "D"; break;
            case Polarization::antidiagonal: pol_str = "A"; break;
            case Polarization::RCP: pol_str = "RCP"; break;
            case Polarization::LCP: pol_str = "LCP"; break;
        }

        std::cout << std::setw(3) << i
                  << " |  " << static_cast<int>(sp.qubit.bit)
                  << "  |   " << basis_str
                  << "   |      " << std::setw(3) << pol_str
                  << "        |  " << std::setw(5) << sp.pulse.count_photons
                  << "   |  " << std::fixed << std::setprecision(6) << sp.pulse.timestamp * 1e9
                  << "\n";
    }

    // --- Шаг 5: Алиса "отправляет" (поляризации) ---
    auto states = alice.send(N);

    // --- Шаг 6: Боб принимает и измеряет ---
    auto bob_results = bob.receive(states);

    std::cout << "\n[BOB] Measurement results:\n";
    const auto& bob_bases = bob.get_bases();
    for (size_t i = 0; i < bob_results.size(); ++i) {
        std::string basis_str = (bob_bases[i].basis == Basis::rectilinear) ? "R" : "D";
        std::string bit_str = bob_results[i].has_value()
                                ? std::to_string(static_cast<int>(bob_results[i].value()))
                                : "X"; // неопределён
        std::cout << "Idx " << std::setw(2) << i
                  << " | Basis: " << basis_str
                  << " | Bit: " << bit_str
                  << "\n";
    }

    // --- Шаг 7: Согласование ключей ---
    auto key = sift_key(alice, bob, bob_results);

    std::cout << "\n[SIFTED KEY] ";
    if (key.empty()) {
        std::cout << "(пустой — нет совпадений базисов)\n";
    } else {
        for (auto bit : key)
            std::cout << static_cast<int>(bit);
        std::cout << "\n";
    }

    std::cout << "End" << std::endl;
    return 0;
}