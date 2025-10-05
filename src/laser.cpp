#include "laser.hpp"
#include <random>

static uint16_t get_photons(double photon_dist) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::poisson_distribution<int> dist(photon_dist);
    return static_cast<uint16_t>(dist(gen));
}

static uint16_t poisson_draw(double mean) {
    static thread_local std::random_device rd;
    static thread_local std::mt19937 gen(rd());
    std::poisson_distribution<int> dist(mean);
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

    uint16_t count = poisson_draw(mean_photons);

    Pulse pulse(count, Polarization::horizontal, pulse_duration_s, current_time);

    current_time += 1.0 / repeat_rate_hz;

    return pulse;
}