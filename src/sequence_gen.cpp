#include "sequence_generator.hpp"

SequenceGenerator::SequenceGenerator(unsigned int seed)
    : gen(seed), bit_dist(0, 1), basis_dist(0, 1) {}

void SequenceGenerator::generate(size_t length) {
    sequence.clear();
    sequence.reserve(length);

    for (size_t i = 0; i < length; ++i) {
        Bit bit   = static_cast<Bit>(bit_dist(gen));
        Basis basis = (basis_dist(gen) == 0) ? Basis::rectilinear : Basis::diagonal;

        sequence.push_back(QubitPulse{bit, basis, time_counter++});
    }
}

const std::vector<QubitPulse>& SequenceGenerator::get_sequence() const {
    return sequence;
}

const QubitPulse& SequenceGenerator::operator[](size_t idx) const {
    return sequence[idx];
}

size_t SequenceGenerator::size() const {
    return sequence.size();
}