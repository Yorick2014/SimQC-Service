#include "laser.hpp"

uint16_t get_photons(double photon_dist);

Pulse LaserDevice::gen_pulse(double photon_dist) {
    uint16_t c_p = get_photons(photon_dist);

    return Pulse(c_p, Polarization::horizontal, 1.0);
}

uint16_t get_photons(double photon_dist){
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::poisson_distribution<int> dist(photon_dist);
    return dist(gen);
}