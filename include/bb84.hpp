#pragma once

#include <vector>
#include <optional>
#include "sequence_generator.hpp"
#include "modulator.hpp"
#include "laser.hpp"

struct SentPulse {
    Pulse pulse;
    Qubit qubit;
    SentPulse(const Pulse& p, const Qubit& q) : pulse(p), qubit(q) {}
};

class Alice {
private:
    SequenceGenerator generator;
    PolarizationModulator modulator;
    ILaser& laser;                // ссылочный внешний лазер (передаётся извне)

public:
    // Принимаем внешний ILaser, чтобы можно было переиспользовать разные лазеры.
    explicit Alice(ILaser& laser_ref, unsigned int seed = std::random_device{}());

    // Сгенерировать пары {Pulse + Qubit} (не отправляет),
    // устанавливает поляризацию в Pulse в соответствии с Qubit.
    std::vector<SentPulse> generate_pulses(size_t length);

    // Отправить (логически) — возвращает вектор поляризаций (для совместимости / тестов).
    // Реально отправку в квантовый канал предполагаем делать вне — передаём вектор SentPulse.
    std::vector<Polarization> send(size_t length);

    // Получить последовательность битов и базисов Алисы
    const std::vector<Qubit>& get_sequence() const;
};

class Bob {
private:
    SequenceGenerator basis_generator; // для случайных базисов

public:
    explicit Bob(unsigned int seed = std::random_device{}());

    std::vector<std::optional<Bit>> receive(const std::vector<Polarization>& states);

    // Получить использованные базисы Боба
    const std::vector<Qubit>& get_bases() const;
};

// Согласование ключей (оставить только те позиции, где базисы совпали)
std::vector<Bit> sift_key(const Alice& alice, const Bob& bob,
                          const std::vector<std::optional<Bit>>& bob_results);

