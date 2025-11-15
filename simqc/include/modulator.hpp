#pragma once

#include <cmath>
#include <complex>
#include <stdexcept>
#include "simulation_params.hpp"

using JonesVector = std::array<std::complex<double>, 2>;

class IModulator {
public:
    virtual ~IModulator() = default;

    virtual JonesVector modulate(const Qubit& qubit) const = 0;

    virtual Polarization to_polarization_bb84(const Qubit& qubit) const = 0;
};

class PolarizationModulator : public IModulator {
public:
    Polarization to_polarization_bb84(const Qubit& qubit) const override;
    JonesVector modulate(const Qubit& qubit) const override;
};

class ModulatorFactory {
public:
    static std::unique_ptr<IModulator> create(const std::string& type);
};

inline std::string polarization_to_string(Polarization pol) {
    switch (pol) {
        case Polarization::horizontal:      return "horizontal";
        case Polarization::vertical:        return "vertical";
        case Polarization::diagonal:        return "diagonal";
        case Polarization::antidiagonal:    return "antidiagonal";
        case Polarization::RCP:             return "RCP";
        case Polarization::LCP:             return "LCP";
        default:                            return "unknown";
    }
}