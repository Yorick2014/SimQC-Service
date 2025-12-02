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
#include <fstream>
#include <sstream>
#include <algorithm>

inline std::string basis_to_string(Basis b) {
    return (b == Basis::rectilinear) ? "+" : "x";
}

inline std::string bit_to_string(Bit bit) {
    return (bit == Bit::zero) ? "0" : "1";
}

void save_pulses_csv(const std::string& dir,
                     const std::vector<Pulse>& pulses,
                     const std::vector<Qubit>& bob_seq,
                     const std::vector<Bit>& bob_bits,
                     const std::vector<bool>& match_basis,
                     const std::vector<bool>& errors,
                     const std::vector<double>& time_registered_by_id_H,
                     const std::vector<double>& time_registered_by_id_V)
{
    std::ofstream fout(dir + "/pulses.csv");
    fout << "pulse_index,alice_basis,alice_bit,bob_basis,bob_bit,detected,match_basis,is_error\n";

    size_t num_pulses = pulses.size();
    for (size_t i = 0; i < num_pulses; ++i) {
        char detected = 0;
        double regH = (i < time_registered_by_id_H.size()) ? time_registered_by_id_H[i] : 0.0;
        double regV = (i < time_registered_by_id_V.size()) ? time_registered_by_id_V[i] : 0.0;

        if (regH > 0) detected = 'H';
        else if (regV > 0) detected = 'V';

        std::string bob_bit_str = "";
        if (detected && i < bob_bits.size() && match_basis[i]) {
            bob_bit_str = bit_to_string(bob_bits[i]);
        }

        fout << i << ","
             << basis_to_string(pulses[i].basis) << ","
             << bit_to_string(pulses[i].bit) << ","
             << basis_to_string(bob_seq[i].basis) << ","
             << bob_bit_str << ","
             << detected << ","
             << (match_basis[i] ? "1" : "0") << ","
             << (errors[i] ? "1" : "0") << "\n";
    }
    fout.close();
}

void save_stats_csv(const std::string& dir,
                    size_t total_pulses,
                    int detected_H,
                    int detected_V,
                    size_t raw_key_len,
                    size_t sifted_key_len,
                    double QBER,
                    double key_rate,
                    const std::vector<Bit>& sifted_key,
                    const Common& params,
                    const LaserData& laser,
                    const QuantumChannelData& qchan,
                    const PhotodetectorData& pd)
{
    std::ofstream fout(dir + "/stats.csv");

    fout << "total_pulses,detected_H,detected_V,raw_key_len,sifted_key_len,QBER,key_rate,sifted_key\n";
    fout << total_pulses << ","
         << detected_H << ","
         << detected_V << ","
         << raw_key_len << ","
         << sifted_key_len << ","
         << QBER << ","
         << key_rate << ",";

    for (auto b : sifted_key) fout << bit_to_string(b);
    fout << "\n\n";

    // -------- input parameters --------
    fout << "[input_parameters]\n";

    // Common
    fout << "protocol," << params.protocol << "\n";
    fout << "laser_type," << params.laser_type << "\n";
    fout << "modulator_type," << params.modulator_type << "\n";
    fout << "channel_type," << params.channel_type << "\n";
    fout << "photodetector_type," << params.photodetector_type << "\n";
    fout << "num_pulses," << params.num_pulses << "\n";
    fout << "seed_Alice," << params.seed_Alice << "\n";
    fout << "seed_Bob," << params.seed_Bob << "\n";

    // Laser
    fout << "central_wavelength," << laser.central_wavelength << "\n";
    fout << "laser_power_w," << laser.laser_power_w << "\n";
    fout << "pulse_duration," << laser.pulse_duration << "\n";
    fout << "attenuation_db," << laser.attenuation_db << "\n";
    fout << "repeat_rate," << laser.repeat_rate << "\n";

    // Quantum channel
    fout << "channel_length," << qchan.channel_length << "\n";
    fout << "chromatic_dispersion," << qchan.chromatic_dispersion << "\n";
    fout << "channel_attenuation," << qchan.channel_attenuation << "\n";
    fout << "is_att," << (qchan.is_att ? "true" : "false") << "\n";
    fout << "is_crom_disp," << (qchan.is_crom_disp ? "true" : "false") << "\n";

    // Photodetector
    fout << "pde," << pd.pde << "\n";
    fout << "dead_time," << pd.dead_time << "\n";
    fout << "dcr," << pd.dcr << "\n";
    fout << "time_slot," << pd.time_slot << "\n";
    fout << "afterpulse_prob," << pd.afterpulse_prob << "\n";
    fout << "afterpulse_delay," << pd.afterpulse_delay << "\n";

    fout.close();
}


std::vector<Pulse> TestBB84::send_pulse(ILaser& laser,
                                        IModulator& modulator,
                                        Common& params,
                                        LaserData& laser_data)
{
    SequenceGenerator seq_gen(params.seed_Alice);
    seq_gen.generate(params.num_pulses);
    const auto& seq = seq_gen.get_sequence();

    std::vector<Pulse> out;
    out.reserve(params.num_pulses);

    for (size_t i = 0; i < params.num_pulses; ++i) {
        Pulse p = laser.generate_pulse();
        const Qubit& q = seq[i];

        p.polarization = modulator.to_polarization_bb84(q);
        p.basis        = q.basis;
        p.bit          = q.bit;

        p.id = i;

        out.push_back(p);
    }

    return out;
}

void TestBB84::run(Common& params,
                   LaserData& laser_data,
                   QuantumChannelData& q_channel_data,
                   PhotodetectorData& ph_data,
                   const std::string& run_id)
{
    std::cout << "--- Start BB84 for run_id: " << run_id << " ---" << std::endl;

    std::ostringstream folder_name;
    folder_name << "results/bb84_" << run_id;
    std::filesystem::create_directories(folder_name.str());
    std::string run_dir = folder_name.str();

    // --- Алиса и Боб ---
    auto laser = LaserFactory::create(params.laser_type, laser_data);
    auto alice_modulator = ModulatorFactory::create(params.modulator_type);
    auto bob_modulator   = ModulatorFactory::create(params.modulator_type);

    // --- генерируем импульсы Алисы ---
    std::vector<Pulse> pulses = send_pulse(*laser, *alice_modulator, params, laser_data);

    // --- Квантовый канал ---
    auto channel = QuantumChannelFactory::create(params.channel_type, pulses, params, laser_data, q_channel_data);
    std::vector<Pulse> transmitted = channel->transmit();

    // --- Генерация базисов Боба до измерения ---
    SequenceGenerator bob_seq_gen(params.seed_Bob);
    bob_seq_gen.generate(params.num_pulses);
    const auto& bob_seq = bob_seq_gen.get_sequence();

    std::vector<Pulse> channel_H; channel_H.reserve(transmitted.size());
    std::vector<Pulse> channel_V; channel_V.reserve(transmitted.size());

    std::mt19937 gen(std::random_device{}());
    std::uniform_real_distribution<double> dist01(0.0, 1.0);

    for (const auto& p : transmitted) {
        size_t id = p.id;
        if (id >= bob_seq.size()) continue;

        Basis bob_basis = bob_seq[id].basis;
        double p_first = 0.5;
        double p_second = 0.5;

        if (bob_basis == Basis::rectilinear) {
            if (p.polarization == Polarization::horizontal) {
                p_first = 1.0; p_second = 0.0;
            } else if (p.polarization == Polarization::vertical) {
                p_first = 0.0; p_second = 1.0;
            } else {
                p_first = 0.5; p_second = 0.5;
            }
        } else {
            if (p.polarization == Polarization::diagonal) {
                p_first = 1.0; p_second = 0.0;
            } else if (p.polarization == Polarization::antidiagonal) {
                p_first = 0.0; p_second = 1.0;
            } else {
                p_first = 0.5; p_second = 0.5;
            }
        }

        double r = dist01(gen);
        if (r < p_first) {
            Pulse q = p;
            if (bob_basis == Basis::rectilinear) q.polarization = Polarization::horizontal;
            else q.polarization = Polarization::diagonal;
            channel_H.push_back(q);
        } else {
            Pulse q = p;
            if (bob_basis == Basis::rectilinear) q.polarization = Polarization::vertical;
            else q.polarization = Polarization::antidiagonal;
            channel_V.push_back(q);
        }
    }

    auto detector_H = PhotodetectorFactory::create(params.photodetector_type, ph_data);
    auto detector_V = PhotodetectorFactory::create(params.photodetector_type, ph_data);

    std::vector<double> time_slots_H, time_slots_V;
    detector_H->generate_time_slots(channel_H.size(), laser_data.repeat_rate, time_slots_H);
    detector_V->generate_time_slots(channel_V.size(), laser_data.repeat_rate, time_slots_V);

    std::vector<std::vector<double>> photon_times_H, photon_times_V;
    std::vector<unsigned int> num_photons_H, num_photons_V;
    for (auto& p : channel_H) num_photons_H.push_back(p.count_photons);
    for (auto& p : channel_V) num_photons_V.push_back(p.count_photons);

    detector_H->generate_photon_timelabels(channel_H.size(), num_photons_H, photon_times_H, time_slots_H, laser_data.pulse_duration);
    detector_V->generate_photon_timelabels(channel_V.size(), num_photons_V, photon_times_V, time_slots_V, laser_data.pulse_duration);

    std::vector<double> time_registered_H, time_registered_V;
    int num_registered_H = detector_H->register_pulses(photon_times_H, time_slots_H, time_registered_H);
    int num_registered_V = detector_V->register_pulses(photon_times_V, time_slots_V, time_registered_V);

    std::vector<double> time_H_by_id(pulses.size(), 0.0);
    std::vector<double> time_V_by_id(pulses.size(), 0.0);

    std::vector<bool> used_H(time_registered_H.size(), false);
    std::vector<bool> used_V(time_registered_V.size(), false);

    double slot_duration = ph_data.time_slot;

    for (size_t j = 0; j < channel_H.size(); ++j) {
        size_t id = channel_H[j].id;
        if (id >= time_H_by_id.size()) continue;

        double left = time_slots_H[j] - slot_duration / 2.0;
        double right = time_slots_H[j] + slot_duration / 2.0;

        for (size_t k = 0; k < time_registered_H.size(); ++k) {
            if (used_H[k]) continue;
            double t = time_registered_H[k];
            if (t >= left && t <= right) {
                time_H_by_id[id] = t;
                used_H[k] = true;
                break;
            }
        }
    }

    // channel_V
    for (size_t j = 0; j < channel_V.size(); ++j) {
        size_t id = channel_V[j].id;
        if (id >= time_V_by_id.size()) continue;

        double left = time_slots_V[j] - slot_duration / 2.0;
        double right = time_slots_V[j] + slot_duration / 2.0;

        for (size_t k = 0; k < time_registered_V.size(); ++k) {
            if (used_V[k]) continue;
            double t = time_registered_V[k];
            if (t >= left && t <= right) {
                time_V_by_id[id] = t;
                used_V[k] = true;
                break;
            }
        }
    }

    std::vector<Bit> bob_bits(params.num_pulses, Bit::zero);
    std::vector<bool> match_basis(params.num_pulses, false);
    std::vector<bool> errors(params.num_pulses, false);
    std::vector<Bit> sifted_key;

    for (size_t i = 0; i < pulses.size(); ++i) {
        Bit b_bit = Bit::zero;
        Polarization detected_pol;
        bool detected = false;

        if (i < time_H_by_id.size() && time_H_by_id[i] > 0.0) {
            detected = true;
            if (bob_seq[i].basis == Basis::rectilinear) {
                detected_pol = Polarization::horizontal;
            } else {
                detected_pol = Polarization::diagonal;
            }
        } else if (i < time_V_by_id.size() && time_V_by_id[i] > 0.0) {
            detected = true;
            if (bob_seq[i].basis == Basis::rectilinear) {
                detected_pol = Polarization::vertical;
            } else { 
                detected_pol = Polarization::antidiagonal;
            }
        }

        match_basis[i] = (bob_seq[i].basis == pulses[i].basis);

        if (!detected || !match_basis[i]) continue;

        if (bob_seq[i].basis == Basis::rectilinear) {
            if (detected_pol == Polarization::horizontal) b_bit = Bit::zero;
            else if (detected_pol == Polarization::vertical) b_bit = Bit::one;
        } else { // диагональный
            if (detected_pol == Polarization::diagonal) b_bit = Bit::zero;
            else if (detected_pol == Polarization::antidiagonal) b_bit = Bit::one;
        }
        bob_bits[i] = b_bit;

        if (bob_bits[i] != pulses[i].bit) errors[i] = true;
        sifted_key.push_back(bob_bits[i]);
    }

    size_t total_pulses = pulses.size();
    size_t raw_key_len = num_registered_H + num_registered_V;
    size_t sifted_key_len = sifted_key.size();
    
    double QBER = static_cast<double>(total_pulses - raw_key_len) / total_pulses * 100.0;
    double key_rate = static_cast<double>(sifted_key_len) / total_pulses;

    save_pulses_csv(run_dir, pulses, bob_seq, bob_bits, match_basis, errors, time_H_by_id, time_V_by_id);
    save_stats_csv(run_dir,
               total_pulses,
               num_registered_H,
               num_registered_V,
               raw_key_len,
               sifted_key_len,
               QBER,
               key_rate,
               sifted_key,
               params,
               laser_data,
               q_channel_data,
               ph_data);


    std::cout << "[BB84] Результаты сохранены в " << run_dir << std::endl;
    std::cout << "QBER (%): " << QBER << ", Key rate: " << key_rate << std::endl;
}
