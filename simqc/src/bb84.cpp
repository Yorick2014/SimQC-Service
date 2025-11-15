#include "bb84.hpp"
#include "laser.hpp"
#include "modulator.hpp"
#include "quantum_channel.hpp"
#include "photodetector.hpp"
#include "beam_splitter.hpp"

#include <iostream>
#include <memory>
#include <utility>
#include <iomanip>
#include <filesystem>
#include <optional>
#include <random>
#include <chrono>
#include <sstream>
#include <fstream>

inline std::string basis_to_string(Basis b) {
    switch (b) {
        case Basis::rectilinear: return "R";
        case Basis::diagonal:    return "D";
        default: return "Unknown";
    }
}

inline std::string bit_to_string(Bit bit) {
    switch (bit) {
        case Bit::zero: return "0";
        case Bit::one:  return "1";
        default: return "Unknown";
    }
}

void save_results_to_csv(const std::string& base_dir,
                         const Common& params,
                         const LaserData& laser_data,
                         const QuantumChannelData& channel_data,
                         const PhotodetectorData& det_data,
                         const std::vector<Pulse>& pulses,
                         const std::vector<double>& registered_H,
                         const std::vector<double>& registered_V,
                         const std::vector<Basis>& bob_bases,
                         const std::vector<std::optional<Bit>>& bob_results,
                         const std::vector<Bit>& sifted_key)
{
    namespace fs = std::filesystem;
    fs::create_directories(base_dir);

    auto now = std::chrono::system_clock::now();
    auto now_time_t = std::chrono::system_clock::to_time_t(now);
    std::tm tm = *std::localtime(&now_time_t);

    std::ostringstream fname;
    fname << base_dir << "/bb84_"
          << std::put_time(&tm, "%Y%m%d_%H%M%S")
          << ".csv";

    std::ofstream fout(fname.str());
    if (!fout.is_open()) {
        std::cerr << "Не удалось создать файл: " << fname.str() << std::endl;
        return;
    }

    fout << "index,alice_basis,bob_basis,match,alice_bit,bob_result,registered_H,registered_V,polarization\n";
    for (size_t i = 0; i < pulses.size(); ++i) {
        bool match = (pulses[i].basis == bob_bases[i]);
        bool registered = bob_results[i].has_value();

        fout << i << ","
             << basis_to_string(pulses[i].basis) << ","
             << basis_to_string(bob_bases[i]) << ","
             << (match ? "Y" : "N") << ","
             << static_cast<int>(pulses[i].bit) << ","
             << (registered ? std::to_string(static_cast<int>(bob_results[i].value())) : "-") << ","
             << (i < registered_H.size() ? registered_H[i] : 0.0) << ","
             << (i < registered_V.size() ? registered_V[i] : 0.0) << ","
             << polarization_to_string(pulses[i].polarization)
             << "\n";
    }

    // Просеянный ключ
    fout << "\n# Sifted key\n";
    for (auto b : sifted_key) fout << static_cast<int>(b);
    fout << "\n";

    // Входные параметры
    fout << "\n# Input parameters\n";
    fout << "num_pulses," << params.num_pulses << "\n";
    fout << "seed_Alice," << params.seed_Alice << "\n";
    fout << "seed_Bob," << params.seed_Bob << "\n";
    fout << "laser_type," << laser_data.central_wavelength << "\n";
    fout << "laser_power," << laser_data.laser_power_w << "\n";
    fout << "attenuation_db," << laser_data.attenuation_db << "\n";
    fout << "pulse_duration," << laser_data.pulse_duration << "\n";
    fout << "repeat_rate," << laser_data.repeat_rate << "\n";
    fout << "channel_length," << channel_data.channel_length << "\n";
    fout << "chromatic_dispersion," << channel_data.chromatic_dispersion << "\n";
    fout << "channel_attenuation," << channel_data.channel_attenuation << "\n";
    fout << "pde," << det_data.pde << "\n";

    fout.close();
    std::cout << "[CSV] Результаты сохранены в " << fname.str() << std::endl;
}

std::vector<Pulse> BB84::send_pulses(ILaser& laser, IModulator& modulator, Common& params, LaserData& laser_data)
{
    SequenceGenerator seq_generator(params.seed_Alice);
    seq_generator.generate(params.num_pulses);
    const auto& seq = seq_generator.get_sequence();

    std::vector<Pulse> out;
    out.reserve(params.num_pulses);    

    for (size_t i = 0; i < params.num_pulses; ++i) {
        Pulse p = laser.generate_pulse();
        const Qubit& q = seq.at(i);

        p.polarization = modulator.to_polarization_bb84(q);
        p.basis = q.basis;
        p.bit   = q.bit;

        out.push_back(p);
    }

    return out;
}

std::vector<std::optional<Bit>> measure_pulses(){
    
}

void BB84::run(Common& params, LaserData& laser_data, QuantumChannelData& q_channel_data, PhotodetectorData& ph_data)
{
    std::cout << "--- Start BB84 ---" << std::endl;

    // Лазер и модуляторы Алисы и Боба
    auto laser = LaserFactory::create(params.laser_type, laser_data);
    auto alice_modulator = ModulatorFactory::create(params.modulator_type);
    auto bob_modulator   = ModulatorFactory::create(params.modulator_type);

    // Генерация импульсов Алисы
    std::vector<Pulse> pulses = send_pulses(*laser, *alice_modulator, params, laser_data);

    // Базы и биты Алисы
    std::vector<Basis> alice_bases(params.num_pulses);
    std::vector<Bit>   alice_bits(params.num_pulses);
    for (size_t i = 0; i < params.num_pulses; ++i) {
        alice_bases[i] = pulses[i].basis;
        alice_bits[i]  = pulses[i].bit;
    }

    // Квантовый канал
    auto channel = QuantumChannelFactory::create(params.channel_type, pulses, params, laser_data, q_channel_data);
    std::vector<Pulse> transmitted = channel->transmit();

    // PBS сплиттер
    PBS splitter;
    auto [channel_H, channel_V] = splitter.split(transmitted);

    // Фотодетекторы
    auto detector_H = PhotodetectorFactory::create(params.photodetector_type, ph_data);
    auto detector_V = PhotodetectorFactory::create(params.photodetector_type, ph_data);

    // Измерение каждого импульса Бобом
    std::mt19937 gen(params.seed_Bob);
    std::uniform_int_distribution<int> basis_dist(0, 1);

    std::vector<Basis> bob_bases(params.num_pulses);
    std::vector<std::optional<Bit>> bob_results(params.num_pulses, std::nullopt);

    for (size_t i = 0; i < params.num_pulses; ++i) {
        // Случайный базис Боба
        bob_bases[i] = (basis_dist(gen) == 0) ? Basis::rectilinear : Basis::diagonal;

        // Проверяем совпадение базисов
        if (bob_bases[i] != alice_bases[i]) continue;

        // Детектируем фотон на соответствующем канале
        bool detected = false;
        if (pulses[i].polarization == Polarization::horizontal) {
            detected = detector_H->detect(transmitted[i]);
            if (detected) bob_results[i] = Bit::zero;
        } else if (pulses[i].polarization == Polarization::vertical) {
            detected = detector_V->detect(transmitted[i]);
            if (detected) bob_results[i] = Bit::one;
        }
    }

    std::vector<Bit> sifted_key;
    for (size_t i = 0; i < params.num_pulses; ++i) {
        if (bob_results[i].has_value())
            sifted_key.push_back(bob_results[i].value());
    }

    std::cout << "\n--- Логи BB84 ---\n";
    std::cout << "Индекс | Alice Basis | Bob Basis | Match | Alice Bit | Bob Result\n";
    for (size_t i = 0; i < params.num_pulses; ++i) {
        bool match = (alice_bases[i] == bob_bases[i]);
        std::cout << i << " | "
                  << basis_to_string(alice_bases[i]) << " | "
                  << basis_to_string(bob_bases[i]) << " | "
                  << (match ? "Y" : "N") << " | "
                  << static_cast<int>(alice_bits[i]) << " | "
                  << (bob_results[i].has_value() ? std::to_string(static_cast<int>(bob_results[i].value())) : "-")
                  << "\n";
    }

    std::cout << "\nСогласованный ключ: ";
    for (auto b : sifted_key) std::cout << static_cast<int>(b);
    std::cout << "\n";

    save_results_to_csv("results", params, laser_data, q_channel_data, ph_data,
                        pulses, {}, {}, bob_bases, bob_results, sifted_key);
}