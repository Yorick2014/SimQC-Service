#pragma once
#include <vector>
#include "simulation_params.hpp"

class IQuantumChannel {
public:
    virtual ~IQuantumChannel() = default;
    virtual std::vector<Pulse> transmit() = 0;
};

class QuantumChannelFactory {
public:
    static std::unique_ptr<IQuantumChannel> create(
        const std::string& type,
        const std::vector<Pulse>& pulses,
        Common& params,
        LaserData& laser_data,
        const QuantumChannelData& channel
    );
};

class FOCL : public IQuantumChannel {
private:
    std::vector<Pulse> pulses_;
    Common& params_;
    LaserData& laser_data_;
    QuantumChannelData channel_;

    double get_att(double att_db, double length_km) const;
    bool is_photon_loss(double random_val, double loss_prob) const;
    double compute_chromatic_broadening() const;

public:
    FOCL(const std::vector<Pulse>& pulses,
                       Common& params,
                       LaserData& laser_data,
                       const QuantumChannelData& channel);

    std::vector<Pulse> transmit() override;
};
