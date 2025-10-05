#include "modulator.hpp"

Polarization PolarizationModulator::to_polarization(const Qubit& qubit) const {
    if (qubit.basis == Basis::rectilinear) {
        return (qubit.bit == Bit::zero) ? Polarization::horizontal
                                        : Polarization::vertical;
    } else if (qubit.basis == Basis::diagonal) {
        return (qubit.bit == Bit::zero) ? Polarization::diagonal
                                        : Polarization::antidiagonal;
    }
    throw std::runtime_error("Unknown basis");
}

JonesVector PolarizationModulator::modulate(const Qubit& qubit) const {
    switch (to_polarization(qubit)) {
        case Polarization::horizontal:     return {1.0, 0.0};
        case Polarization::vertical:       return {0.0, 1.0};
        case Polarization::diagonal:       return {1.0 / std::sqrt(2), 1.0 / std::sqrt(2)};
        case Polarization::antidiagonal:   return {1.0 / std::sqrt(2), -1.0 / std::sqrt(2)};
        case Polarization::RCP:            return {1.0 / std::sqrt(2), {0.0, 1.0 / std::sqrt(2)}};
        case Polarization::LCP:            return {1.0 / std::sqrt(2), {0.0, -1.0 / std::sqrt(2)}};
    }
    throw std::runtime_error("Unknown polarization");
}
