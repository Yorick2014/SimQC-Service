#pragma once
#include <vector>
#include <utility>
#include "simulation_params.hpp"

class IBeamSplitter {
public:
    virtual ~IBeamSplitter() = default;
    virtual std::pair<std::vector<Pulse>, std::vector<Pulse>>
    split(const std::vector<Pulse>& pulses) const = 0;
};

class PBS : public IBeamSplitter {
private:
    double cross_talk_prob_; // вероятность ошибочного разделения

    static double random_0_to_1();

public:
    explicit PBS(double cross_talk_prob = 0.01);

    std::pair<std::vector<Pulse>, std::vector<Pulse>>
    split(const std::vector<Pulse>& pulses) const override;
};
