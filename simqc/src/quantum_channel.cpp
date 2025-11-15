#include "quantum_channel.hpp"
#include <cmath>
#include <cstdlib>
#include <iostream>

std::unique_ptr<IQuantumChannel> QuantumChannelFactory::create(const std::string& type,
                                                              const std::vector<Pulse>& pulses,
                                                              Common& params,
                                                              LaserData& laser_data,
                                                              const QuantumChannelData& channel) {
    if (type == "FOCL" || type == "fiber") {
        std::cout << "[Factory] Создан оптоволоконный канал" << std::endl;
        return std::make_unique<FOCL>(pulses, params, laser_data, channel);
    }
    // сюда можно будет добавить другие типы, например:
    // else if (type == "free_space") return std::make_unique<FreeSpaceChannel>(...);

    throw std::invalid_argument("Неизвестный тип квантового канала: " + type);
}

double FOCL::get_att(double att_db, double length_km) const {
    // перевод затухания в вероятность потери (0..1)
    return 1.0 - std::pow(10.0, -0.1 * att_db * length_km);
}

bool FOCL::is_photon_loss(double random_val, double loss_prob) const {
    return random_val < loss_prob;
}

double FOCL::compute_chromatic_broadening() const {
    const double delta_lambda_nm = 0.1; // типичная ширина спектра лазера, нм
    double tau_ps = delta_lambda_nm * channel_.chromatic_dispersion * channel_.channel_length; // пс
    return tau_ps * 1e-12; // в секунды
}

FOCL::FOCL(const std::vector<Pulse>& pulses,
                                       Common& params,
                                       LaserData& laser_data,
                                       const QuantumChannelData& channel)
    : pulses_(pulses), params_(params), laser_data_(laser_data), channel_(channel) {}


std::vector<Pulse> FOCL::transmit() {
    std::vector<Pulse> transmitted;
    transmitted.reserve(pulses_.size());

    double loss_prob = channel_.is_att
        ? get_att(channel_.channel_attenuation, channel_.channel_length)
        : 0.0;

    double broadening = channel_.is_crom_disp
        ? compute_chromatic_broadening()
        : 0.0;

    for (const auto& p : pulses_) {
        double rand_val = static_cast<double>(std::rand()) / RAND_MAX;
        bool lost = is_photon_loss(rand_val, loss_prob);

        if (!lost) {
            Pulse new_p = p;
            new_p.duration += broadening;
            transmitted.push_back(new_p);
        }
    }

    std::cout << "[FOCLQuantumChannel] Передано " << transmitted.size()
              << " из " << pulses_.size() << " импульсов ("
              << (100.0 * transmitted.size() / pulses_.size()) << "%)\n";

    return transmitted;
}
