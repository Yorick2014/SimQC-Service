#pragma once
#include <vector>
#include <random>
#include "simulation_params.hpp"

class SequenceGenerator {
private:
    std::vector<Qubit> sequence;
    std::mt19937 gen;
    std::uniform_int_distribution<int> bit_dist;
    std::uniform_int_distribution<int> basis_dist;

public:
    SequenceGenerator(unsigned int seed = std::random_device{}());

    void generate(uint32_t length);

    const std::vector<Qubit>& get_sequence() const;
    const Qubit& operator[](uint32_t idx) const;

    uint32_t size() const;
};
