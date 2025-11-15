#include "modulator.hpp"

std::unique_ptr<IModulator> ModulatorFactory::create(const std::string& type){
    
    if (type == "polarization") return std::make_unique<PolarizationModulator>();
    
    throw std::invalid_argument("Неизвестный тип модуятора: " + type);
}

Polarization PolarizationModulator::to_polarization_bb84(const Qubit& qubit) const {
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
    switch (to_polarization_bb84(qubit)) {
        case Polarization::horizontal:     return {1.0, 0.0};
        case Polarization::vertical:       return {0.0, 1.0};
        case Polarization::diagonal:       return {1.0 / std::sqrt(2), 1.0 / std::sqrt(2)};
        case Polarization::antidiagonal:   return {1.0 / std::sqrt(2), -1.0 / std::sqrt(2)};
        case Polarization::RCP:            return {1.0 / std::sqrt(2), {0.0, 1.0 / std::sqrt(2)}};
        case Polarization::LCP:            return {1.0 / std::sqrt(2), {0.0, -1.0 / std::sqrt(2)}};
    }
    throw std::runtime_error("Unknown polarization");
}
