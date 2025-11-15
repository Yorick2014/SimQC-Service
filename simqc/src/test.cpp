#include "test.hpp"
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

void save_results_to_csv(const std::string& base_dir,
                         const Common& params,
                         const LaserData& laser_data,
                         const QuantumChannelData& channel_data,
                         const PhotodetectorData& det_data,
                         const std::vector<Pulse>& pulses,
                         const std::vector<double>& registered_H,
                         const std::vector<double>& registered_V) 
{
    // создаём папку results если её нет
    std::filesystem::create_directories(base_dir);

    // формируем имя файла с timestamp
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

    // Заголовки
    fout << "pulse_index,count_photons,duration,timestamp,polarization,registered_H,registered_V\n";

    size_t num_pulses = pulses.size();
    for (size_t i = 0; i < num_pulses; ++i) {
        const auto& p = pulses[i];
        // проверка наличия зарегистрированных фотонов
        double regH = (i < registered_H.size()) ? registered_H[i] : 0.0;
        double regV = (i < registered_V.size()) ? registered_V[i] : 0.0;

        fout << i << ","
             << p.count_photons << ","
             << p.duration << ","
             << p.timestamp << ","
             << polarization_to_string(p.polarization) << ","
             << regH << ","
             << regV << "\n";
    }

    // Можно также добавить отдельный блок с входными параметрами
    fout << "\n# Входные параметры\n";
    fout << "num_pulses," << params.num_pulses << "\n";
    fout << "seed_Alice," << params.seed_Alice << "\n";
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

std::vector<Pulse> TestBB84::send_pulse(ILaser& laser, IModulator& modulator, Common& params, LaserData& laser_data)
{
    auto num_pulses = params.num_pulses;

    SequenceGenerator seq_generator(params.seed_Alice);
    seq_generator.generate(num_pulses);
    const auto& seq = seq_generator.get_sequence();

    std::vector<Pulse> out;
    out.reserve(num_pulses);    

    for (size_t i = 0; i < num_pulses; ++i) {
        Pulse p = laser.generate_pulse();
        const Qubit& q = seq.at(i);

        p.polarization = modulator.to_polarization_bb84(q);

        out.push_back(p);
    }

    return out;
}

void TestBB84::run(Common& params, LaserData& laser_data, QuantumChannelData& q_channel_data, PhotodetectorData& ph_data)
{
    std::cout << "--- Start BB84 ---" << std::endl;

    // Создаём лазер и модуляторы Алисы и Боба
    auto laser = LaserFactory::create(params.laser_type, laser_data);
    auto alice_modulator = ModulatorFactory::create(params.modulator_type);
    auto bob_modulator   = ModulatorFactory::create(params.modulator_type);

    // Генерация импульсов через лазер + модулятор Алисы
    std::vector<Pulse> pulses = send_pulse(*laser, *alice_modulator, params, laser_data);

    // Квантовый канал
    auto channel = QuantumChannelFactory::create(params.channel_type, pulses, params, laser_data, q_channel_data);
    std::vector<Pulse> transmitted = channel->transmit();

    // PBS сплиттер
    PBS splitter; // default cross_talk_prob
    auto [channel_H, channel_V] = splitter.split(transmitted);

    // Два фотодетектора SPAD
    auto detector_H = PhotodetectorFactory::create(params.photodetector_type, ph_data);
    auto detector_V = PhotodetectorFactory::create(params.photodetector_type, ph_data);

    // Генерация временных слотов для детекторов
    std::vector<double> time_slots_H, time_slots_V;
    detector_H->generate_time_slots(channel_H.size(), laser_data.repeat_rate, time_slots_H);
    detector_V->generate_time_slots(channel_V.size(), laser_data.repeat_rate, time_slots_V);


    // Генерация меток времени фотонов
    std::vector<std::vector<double>> photon_times_H, photon_times_V;
    std::vector<unsigned int> num_photons_H, num_photons_V;

    num_photons_H.reserve(channel_H.size());
    for (auto& p : channel_H) num_photons_H.push_back(p.count_photons);

    num_photons_V.reserve(channel_V.size());
    for (auto& p : channel_V) num_photons_V.push_back(p.count_photons);

    detector_H->generate_photon_timelabels(channel_H.size(), num_photons_H, photon_times_H, time_slots_H, laser_data.pulse_duration);
    detector_V->generate_photon_timelabels(channel_V.size(), num_photons_V, photon_times_V, time_slots_V, laser_data.pulse_duration);

    // Регистрация импульсов
    std::vector<double> registered_H, registered_V;
    int num_registered_H = detector_H->register_pulses(photon_times_H, time_slots_H, registered_H);
    int num_registered_V = detector_V->register_pulses(photon_times_V, time_slots_V, registered_V);

    save_results_to_csv("results", params, laser_data, q_channel_data, ph_data, pulses, registered_H, registered_V);

    // Вывод информации
    std::cout << "Всего импульсов: " << pulses.size() << std::endl;
    std::cout << "Регистрация H: " << num_registered_H << " фотонов" << std::endl;
    std::cout << "Регистрация V: " << num_registered_V << " фотонов" << std::endl;
}