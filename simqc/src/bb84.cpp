#include "bb84.hpp"
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <ctime>

AliceBB84::AliceBB84(ILaser& laser_ref, unsigned int seed)
    : generator(seed), modulator(), laser(laser_ref) {}

// generate_pulses: генерируем последовательность Qubit'ов через SequenceGenerator,
// создаём соответствующее количество физических импульсов через ILaser,
// задаём каждому Pulse поляризацию согласно протоколу (через PolarizationModulator)
// и возвращаем вектор пар {Pulse, Qubit}.
std::vector<SentPulse> AliceBB84::generate_pulses(size_t length) {
    generator.generate(length);
    const auto& seq = generator.get_sequence();

    std::vector<SentPulse> out;
    out.reserve(length);

    for (size_t i = 0; i < length; ++i) {
        Pulse p = laser.generate_pulse();
        const Qubit& q = seq.at(i);

        // поляризация согласно BB84
        p.polarization = modulator.to_polarization(q);

        out.emplace_back(p, q);
    }

    return out;
}

// send: упрощённо — вызывает generate_pulses и возвращает вектор поляризаций,
// которые подлежат отправке в квантовый канал.
// Фактическая "передача" (например, помещение в объект QuantumChannel) должна
// выполняться в вызывающем коде с использованием результата generate_pulses().
std::vector<Polarization> AliceBB84::send(size_t length) {
    auto sent = generate_pulses(length);
    std::vector<Polarization> pols;
    pols.reserve(sent.size());
    for (const auto& sp : sent) {
        pols.push_back(sp.pulse.polarization);
    }
    return pols;
}

const std::vector<Qubit>& AliceBB84::get_sequence() const {
    return generator.get_sequence();
}

BobBB84::BobBB84(unsigned int seed)
    : basis_generator(seed) {}

std::vector<std::optional<Bit>> BobBB84::receive(const std::vector<Polarization>& states) {
    size_t n = states.size();
    basis_generator.generate(n);
    const auto& bases = basis_generator.get_sequence();

    std::vector<std::optional<Bit>> results;
    results.reserve(n);

    for (size_t i = 0; i < n; ++i) {
        // Если выбранный базис соответствует поляризации, получаем бит, иначе результат неопределён
        // Тут нужно иметь логику измерения: для простоты — сравниваем ожидаемую поляризацию базиса с полученной.
        // Предположительная функция: PolarizationModulator::measure(polarization, basis) -> optional<Bit>
        // Но её у нас нет — поэтому сделаем простую эвристику (требует замены реальной моделью измерения).
        const Qubit& qb = bases.at(i);
        // Преобразуем базис + предполагаемый нул/единицу в поляризацию — как в модуле Алисы
        Polarization expected0 = Polarization::horizontal;
        Polarization expected1 = Polarization::vertical;
        if (qb.basis == Basis::diagonal) {
            expected0 = Polarization::diagonal;
            expected1 = Polarization::antidiagonal;
        }

        if (states[i] == expected0) {
            results.emplace_back(Bit::zero);
        } else if (states[i] == expected1) {
            results.emplace_back(Bit::one);
        } else {
            results.emplace_back(std::nullopt);
        }
    }

    return results;
}

const std::vector<Qubit>& BobBB84::get_bases() const {
    return basis_generator.get_sequence();
}

/// ------------------- sift_key -------------------
std::vector<Bit> sift_key(const AliceBB84& alice, const BobBB84& bob,
                          const std::vector<std::optional<Bit>>& bob_results) {
    const auto& a_seq = alice.get_sequence();
    const auto& b_bases = bob.get_bases();

    if (a_seq.size() != bob_results.size() || b_bases.size() != bob_results.size()) {
        throw std::runtime_error("Размеры последовательностей не совпадают при согласовании ключа");
    }

    std::vector<Bit> key;
    key.reserve(bob_results.size());

    for (size_t i = 0; i < bob_results.size(); ++i) {
        // если базисы совпали и результат измерения определён — добавляем бит
        if (a_seq[i].basis == b_bases[i].basis && bob_results[i].has_value()) {
            key.push_back(bob_results[i].value());
        }
    }

    return key;
}

void run_bb84(Common& params, LaserData& laser_data){
    std::cout << "--- Start BB84 ---" << std::endl;

    // ------ ALICE -----
    std::unique_ptr<ILaser> laser = LaserFactory::create(params.laser_type, laser_data);
    AliceBB84 alice(*laser, params.seed_Alice);

    auto N = params.num_pulses;
    auto sent_pulses = alice.generate_pulses(N);
    auto states = alice.send(N);

    // ------ BOB -----
    BobBB84 bob(params.seed_Bob);
    auto bob_results = bob.receive(states);
    const auto& bob_bases = bob.get_bases();

    // ----- SIFTED -----
    auto key = sift_key(alice, bob, bob_results);
    std::string key_str;
    if (key.empty()) {
        key_str = "пустой — нет совпадений базисов";
    } else {
        for (auto bit : key)
        key_str += std::to_string(static_cast<int>(bit));
    }
    
    // ----- WRITE TO CSV -----
    std::time_t now = std::time(nullptr);
    std::tm local_tm = *std::localtime(&now);

    std::stringstream ss;
    ss << std::put_time(&local_tm, "%Y-%m-%d_%H-%M-%S");
    std::string timestamp = ss.str();
    std::string filename = "bb84_results-" + timestamp + ".csv";

    std::ofstream csv_file(filename);
    if (!csv_file.is_open()) {
        std::cerr << "Не удалось открыть файл для записи" << std::endl;
        return;
    }

    csv_file << "Idx,Alice_Bit,Alice_Basis,Alice_Polarization,Alice_Photons,Alice_Timestamp_ns,Bob_Bit,Bob_Basis,key\n";
    csv_file << ",,,,,,,," << key_str << "\n";

    std::cout << "\n[BB84 RESULTS] Writing to CSV" << std::endl;
    for (size_t i = 0; i < sent_pulses.size(); ++i) {
        const auto& sp = sent_pulses[i];

        std::string alice_basis_str = (sp.qubit.basis == Basis::rectilinear) ? "R" : "D";
        std::string alice_pol_str;
        switch (sp.pulse.polarization) {
            case Polarization::horizontal: alice_pol_str = "H"; break;
            case Polarization::vertical: alice_pol_str = "V"; break;
            case Polarization::diagonal: alice_pol_str = "D"; break;
            case Polarization::antidiagonal: alice_pol_str = "A"; break;
            case Polarization::RCP: alice_pol_str = "RCP"; break;
            case Polarization::LCP: alice_pol_str = "LCP"; break;
            default: alice_pol_str = "Undefined"; break;
        }

        std::string bob_bit_str = bob_results[i].has_value()
                                    ? std::to_string(static_cast<int>(bob_results[i].value()))
                                    : "X"; // неопределён
        std::string bob_basis_str = (bob_bases[i].basis == Basis::rectilinear) ? "R" : "D";

        csv_file << i << ","
                 << static_cast<int>(sp.qubit.bit) << ","
                 << alice_basis_str << ","
                 << alice_pol_str << ","
                 << sp.pulse.count_photons << ","
                 << std::fixed << std::setprecision(6) << sp.pulse.timestamp * 1e9 << ","
                 << bob_bit_str << ","
                 << bob_basis_str
                 << "\n";
    }

    csv_file.close();
    std::cout << "Данные записаны в " + filename << std::endl;
}
