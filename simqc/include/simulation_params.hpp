#pragma once

#include <nlohmann/json.hpp>
#include <fstream>
#include <string>

// используется для загрузки конфига непосредственно из json файла
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

// используется для получения конфига из строки
template<typename T>
T set_config(const std::string& json_str) {
    nlohmann::json json = nlohmann::json::parse(json_str);
    return json.get<T>();
}

enum class QKDProtocol {BB84, SARG04};
enum class Polarization { horizontal, vertical, diagonal, antidiagonal, RCP, LCP };
enum class Basis { rectilinear, diagonal };
enum class Bit { zero = 0, one = 1 };

struct Common{
    uint16_t protocol; // 1 = BB84
    std::string laser_type; // AttLaser
    uint32_t num_pulses; // число импульсов, которые будут отправлены
    uint16_t seed_Alice; // seed для генерации случайной последовательности
    uint32_t seed_Bob; // seed для генерации случайной последовательности
};

struct LaserData{
    double central_wavelength; // метры
    double laser_power_w; // ватты
    double pulse_duration; // секунды
    double attenuation_db; // дБ
    double repeat_rate; // Гц (частота повторения импульсов)
};
struct Pulse {
    uint16_t count_photons;
    Polarization polarization;
    double duration;
    double timestamp;
    Pulse(uint16_t c, Polarization p, double d, double t)
        : count_photons(c), polarization(p), duration(d), timestamp(t) {}
};

struct Qubit {
    Bit bit;
    Basis basis;
};

struct QuantumChannel{
    double channel_length; // км
    double chromatic_dispersion; // пс/(нм·км)
    double channel_attenuation; // дБ
    bool is_att; // учитывать затухание
    bool is_crom_disp; // учитывать хром дисперсию
};
struct Photodetector{
    double quantum_efficiency;
    double dead_time;
    double time_slot;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE (Common,
    protocol,
    laser_type,
    num_pulses,
    seed_Alice,
    seed_Bob
)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(LaserData,
    central_wavelength,
    laser_power_w,
    pulse_duration,
    attenuation_db,
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
