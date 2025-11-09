#include "laser.hpp"
#include <random>
#include <stdexcept>

std::unique_ptr<ILaser> LaserFactory::create(
    const std::string& type,
    const LaserData& data
) {
    if (type == "AttLaser") {
        return std::make_unique<AttLaser>(
            data.central_wavelength,
            data.laser_power_w,
            data.attenuation_db,
            data.pulse_duration,
            data.repeat_rate
        );
    }

    throw std::invalid_argument("Некорректный тип лазера: " + type);
}

static uint16_t get_photons(double photon_dist) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::poisson_distribution<int> dist(photon_dist);
    return static_cast<uint16_t>(dist(gen));
}

double AttLaser::photon_energy_joule(double wavelength_m) {
    const double h = 6.62607015e-34;
    const double c = 299792458.0;
    return h * c / wavelength_m;
}

AttLaser::AttLaser(double wavelength,
                   double laser_power_w,
                   double attenuation_db,
                   double pulse_duration_s,
                   double repeat_rate_hz)
    : wavelength(wavelength),
      laser_power_w(laser_power_w),
      attenuation_db(attenuation_db),
      pulse_duration_s(pulse_duration_s),
      repeat_rate_hz(repeat_rate_hz),
      current_time(0.0) {}

Pulse AttLaser::generate_pulse() {
    const double E_ph = photon_energy_joule(wavelength);

    // линейный коэффициент передачи
    double k = std::pow(10.0, -attenuation_db / 10.0);

    // до ослабления
    double E_pulse = laser_power_w * pulse_duration_s; // в Дж

    // после ослабления
    double mean_photons = k * (E_pulse / E_ph);

    if (mean_photons < 0.0) mean_photons = 0.0;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::poisson_distribution<> distr(mean_photons);

    uint16_t count = distr(gen);
    // std::cout << "num: " << count << std::endl;

    Pulse pulse(count, Polarization::horizontal, pulse_duration_s, current_time);

    current_time += 1.0 / repeat_rate_hz;

    return pulse;
}