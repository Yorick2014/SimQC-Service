#include "bb84.hpp"
#include <stdexcept>
#include <iostream>

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

    std::unique_ptr<ILaser> laser = LaserFactory::create(params.laser_type, laser_data);

    AliceBB84 alice(*laser, params.seed_Alice);
    BobBB84 bob(params.seed_Bob);

    auto N = params.num_pulses;
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
            default: std::cout << "Polarization is undefined"; break;
        }

        std::cout << std::setw(3) << i
                  << " |  " << static_cast<int>(sp.qubit.bit)
                  << "  |   " << basis_str
                  << "   |      " << std::setw(3) << pol_str
                  << "        |  " << std::setw(5) << sp.pulse.count_photons
                  << "   |  " << std::fixed << std::setprecision(6) << sp.pulse.timestamp * 1e9
                  << "\n";
    }

    auto states = alice.send(N);
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

    auto key = sift_key(alice, bob, bob_results);

    std::cout << "\n[SIFTED KEY] ";
    if (key.empty()) {
        std::cout << "(пустой — нет совпадений базисов)\n";
    } else {
        for (auto bit : key)
            std::cout << static_cast<int>(bit);
        std::cout << "\n";
    }
}