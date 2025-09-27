#pragma once
#include <iostream>
#include <random>
#include "simulation_params.hpp"

class LaserDevice {
    Polarization defaultBasis;
    Bit defaultBit;
public:
    Pulse gen_pulse(double photon_dist);
private:

};