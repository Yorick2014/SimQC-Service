#pragma once
#include <vector>
#include <random>
#include "simulation_params.hpp"

class SequenceGenerator {
private:
    std::vector<QubitPulse> sequence;
    std::mt19937 gen;
    std::uniform_int_distribution<int> bit_dist;
    std::uniform_int_distribution<int> basis_dist;
    uint64_t time_counter = 0; // дискретные шаги

public:
    explicit SequenceGenerator(unsigned int seed = std::random_device{}());

    void generate(size_t length);

    const std::vector<QubitPulse>& get_sequence() const;
    const QubitPulse& operator[](size_t idx) const;

    size_t size() const;
};
