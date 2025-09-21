#pragma once

#include <nlohmann/json.hpp>
#include <fstream>
#include <string>

template<typename T>
T load_config(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error("Не удалось открыть файл " + path);
    }

    nlohmann::json json;
    file >> json;

    return json.get<T>();
}

struct Common{
    std::string protocol;
    unsigned int key_length;
    bool automod;
    unsigned int number_keys;
};

struct Laser{
    double central_wavelength;
    double pulse_duration;
    double avg_count_photons;
    double repeat_rate; // частота повторения импульсов
};
struct QuantumChannel{
    double channel_length;
    double chromatic_dispersion;
    double channel_attenuation;
    bool is_att;
    bool is_crom_disp; //хроматическая дисперсия
};
struct Photodetector{
    double quantum_efficiency;
    double dead_time;
    double time_slot;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE (Common,
    protocol,
    key_length,
    automod,
    number_keys
)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Laser,
    central_wavelength,
    pulse_duration,
    avg_count_photons,
    repeat_rate
)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(QuantumChannel,
    channel_length,
    chromatic_dispersion,
    channel_attenuation,
    is_att,
    is_crom_disp
)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Photodetector,
    quantum_efficiency,
    dead_time,
    time_slot
)
