#pragma once
#include <iostream>
#include <random>
#include <memory>
#include <string>
#include "simulation_params.hpp"

class ILaser {
public:
    virtual Pulse generate_pulse() = 0;
    virtual ~ILaser() = default;
};

class LaserFactory {
public:
    static std::unique_ptr<ILaser> create(
        const std::string& type,
        const LaserData& data
    );
};

class AttLaser : public ILaser {
private:
    const double wavelength;   // метры
    double laser_power_w;      // ватт
    double attenuation_db;     // dB (положительное - потеря)
    double pulse_duration_s;   // сек
    double repeat_rate_hz;     // Гц
    double current_time;       // сек

    static double photon_energy_joule(double wavelength_m);

public:
    AttLaser(const double wavelength, double laser_power_w, double attenuation_db, double pulse_duration_s, double repeat_rate_hz);

    Pulse generate_pulse() override;
};
