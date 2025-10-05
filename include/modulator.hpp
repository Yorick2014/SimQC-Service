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

    virtual Polarization to_polarization(const Qubit& qubit) const = 0;
};

class PolarizationModulator : public IModulator {
public:
    Polarization to_polarization(const Qubit& qubit) const override;
    JonesVector modulate(const Qubit& qubit) const override;
};
