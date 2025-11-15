#pragma once
#include "sequence_generator.hpp"
#include "laser.hpp"
#include "modulator.hpp"
#include "quantum_channel.hpp"
#include "beam_splitter.hpp"
#include "photodetector.hpp"

#include <optional>
#include <memory>
#include <utility>
#include <iomanip>
#include <filesystem>
#include <random>
#include <chrono>
#include <sstream>
#include <fstream>

struct BobMeasurementResult {
    std::vector<Basis> bob_bases;
    std::vector<std::optional<Bit>> bob_results;
};

class BB84{
private:
    std::vector<Pulse> send_pulses(ILaser& laser, IModulator& modulator, Common& params, LaserData& laser_data);
    BobMeasurementResult measure_pulses(Common& params, 
                                        const std::vector<Basis>& alice_bases, 
                                        const std::vector<Pulse>& pulses, 
                                        const std::vector<Pulse>& transmitted,
                                        PhotodetectorData& ph_data);

public:
    void run(Common& params, LaserData& laser_data, QuantumChannelData& q_channel_data, PhotodetectorData& ph_data);
};