#include "sequence_generator.hpp"

SequenceGenerator::SequenceGenerator(unsigned int seed)
    : gen(seed), bit_dist(0, 1), basis_dist(0, 1) {}

void SequenceGenerator::generate(uint32_t length) {
    sequence.clear();
    sequence.reserve(length);

    for (uint32_t i = 0; i < length; ++i) {
        Bit bit   = static_cast<Bit>(bit_dist(gen));
        Basis basis = (basis_dist(gen) == 0) ? Basis::rectilinear : Basis::diagonal;

        sequence.push_back(Qubit{bit, basis});
    }
}

const std::vector<Qubit>& SequenceGenerator::get_sequence() const {
    return sequence;
}

const Qubit& SequenceGenerator::operator[](uint32_t idx) const {
    return sequence[idx];
}

uint32_t SequenceGenerator::size() const {
    return sequence.size();
}