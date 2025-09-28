#include "laser.hpp"

uint16_t get_photons(double photon_dist);

Pulse LaserDevice::gen_pulse(double photon_dist, double pulse_duration) {
    uint16_t count_photons = get_photons(photon_dist); // number of photons per pulse

    return Pulse(count_photons, Polarization::horizontal, pulse_duration);
}

uint16_t get_photons(double photon_dist){
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::poisson_distribution<int> dist(photon_dist);
    return dist(gen);
}