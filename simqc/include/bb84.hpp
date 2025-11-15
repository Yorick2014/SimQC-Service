#pragma once
#include "sequence_generator.hpp"
#include "laser.hpp"
#include "modulator.hpp"
#include "quantum_channel.hpp"
#include "beam_splitter.hpp"
#include "photodetector.hpp"

class BB84{
private:
    std::vector<Pulse> send_pulses(ILaser& laser, IModulator& modulator, Common& params, LaserData& laser_data);
    std::vector<std::optional<Bit>> measure_pulses();
public:
    void run(Common& params, LaserData& laser_data, QuantumChannelData& q_channel_data, PhotodetectorData& ph_data);
};