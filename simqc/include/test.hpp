#pragma once

#include "sequence_generator.hpp"
#include "laser.hpp"
#include "modulator.hpp"
#include "quantum_channel.hpp"
#include "beam_splitter.hpp"
#include "photodetector.hpp"

class TestBB84 {
private:
    std::vector<Pulse> send_pulse(ILaser& laser, IModulator& modulator, Common& params, LaserData& laser_data);
public:
    void run(Common& params, 
             LaserData& laser_data, 
             QuantumChannelData& q_channel_data, 
             PhotodetectorData& ph_data,
             const std::string& run_id);
};