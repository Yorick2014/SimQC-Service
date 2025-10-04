#pragma once
#include <iostream>
#include <random>
#include "simulation_params.hpp"

class ILaser {
public:
    virtual Pulse generate_pulse() = 0;
    virtual ~ILaser() = default;
};

class AttLaser : public ILaser {
private:
    const double wavelength;   // метры
    double laser_power_w;      // ватт
    double attenuation_db;     // dB (положительное - потеря)
    double pulse_duration_s;   // сек

    static double photon_energy_joule(double wavelength_m);

public:
    AttLaser(const double wavelength, double laser_power_w, double attenuation_db, double pulse_duration_s = 1e-9);

    Pulse generate_pulse() override;
};