#include "beam_splitter.hpp"
#include <random>

// Конструктор
PBS::PBS(double cross_talk_prob)
    : cross_talk_prob_(cross_talk_prob) {}

std::pair<std::vector<Pulse>, std::vector<Pulse>>
PBS::split(const std::vector<Pulse>& pulses) const {
    std::vector<Pulse> channel_H, channel_V;
    channel_H.reserve(pulses.size());
    channel_V.reserve(pulses.size());

    for (const auto& pulse : pulses) {
        double rand_val = random_0_to_1();

        if (pulse.polarization == Polarization::horizontal) {
            if (rand_val < cross_talk_prob_)
                channel_V.push_back(pulse);
            else
                channel_H.push_back(pulse);
        } else if (pulse.polarization == Polarization::vertical) {
            if (rand_val < cross_talk_prob_)
                channel_H.push_back(pulse);
            else
                channel_V.push_back(pulse);
        } else {
            if (rand_val < 0.5)
                channel_H.push_back(pulse);
            else
                channel_V.push_back(pulse);
        }
    }

    return {channel_H, channel_V};
}

double PBS::random_0_to_1() {
    static thread_local std::mt19937 gen(std::random_device{}());
    static thread_local std::uniform_real_distribution<double> dist(0.0, 1.0);
    return dist(gen);
}
