#include "photodetector.hpp"
#include <cstdlib>
#include <iostream>
#include <ctime>
#include <cmath>

std::unique_ptr<IPhotodetector> PhotodetectorFactory::create(const std::string& type,
                                                            const PhotodetectorData& det_data) {
    if (type == "SPAD" || type == "spad") {
        std::cout << "[Factory] Создан SPAD фотодетектор" << std::endl;
        return std::make_unique<SPAD>(det_data);
    }
    // другие варианты:
    // else if (type == "SNSPD") return std::make_unique<SNSPDDetector>(det_data);

    throw std::invalid_argument("Неизвестный тип фотодетектора: " + type);
}

double SPAD::generate_random_0_to_1() {
    static thread_local std::mt19937 gen(std::random_device{}());
    static thread_local std::uniform_real_distribution<double> dist(0.0, 1.0);
    return dist(gen);
}

bool SPAD::detect(const Pulse& p) {
    for (uint16_t i = 0; i < p.count_photons; ++i) {
        if (generate_random_0_to_1() <= det_data_.pde)
            return true;
    }
    return false;
}

// генерация временных окон регистрации (time slots)
void SPAD::generate_time_slots(unsigned int num_pulses,
                               double rep_rate_hz,
                               std::vector<double>& time_slots) {
    time_slots.clear();
    double step = 1.0 / rep_rate_hz;  // интервал между импульсами (сек)
    time_slots.reserve(num_pulses);
    for (unsigned int i = 0; i < num_pulses; ++i) {
        time_slots.push_back(i * step);
    }
}

// генерация меток времени регистрации фотонов
void SPAD::generate_photon_timelabels(unsigned int num_pulses,
                                      const std::vector<unsigned int>& num_photons,
                                      std::vector<std::vector<double>>& photon_times,
                                      const std::vector<double>& time_slots,
                                      double time_window) {
    photon_times.resize(num_pulses);

    double qe = det_data_.pde;
    if (qe > 1.0) qe /= 100.0; // если задавать в процентах

    for (unsigned int i = 0; i < num_pulses; ++i) {
        photon_times[i].clear();
        photon_times[i].reserve(num_photons[i]);

        for (unsigned int j = 0; j < num_photons[i]; ++j) {
            double rand_num = generate_random_0_to_1();
            if (rand_num <= qe && rand_num > 0.0) {
                // Случайная метка внутри временного окна импульса
                double t = time_slots[i] + rand_num * time_window;
                photon_times[i].push_back(t);
            }
        }

        std::sort(photon_times[i].begin(), photon_times[i].end());
    }
}

// регистрация импульсов (учёт мёртвого времени, DCR, послеимпульсов)
int SPAD::register_pulses(std::vector<std::vector<double>>& photon_times,
                          std::vector<double>& time_slots,
                          std::vector<double>& time_registered) {
    int num_registered = 0;
    double dead_time = det_data_.dead_time;
    double slot_duration = det_data_.time_slot;
    double last_reg_time = -1e9;

    std::mt19937 gen(std::random_device{}());
    std::uniform_real_distribution<double> rand01(0.0, 1.0);

    // вероятность ложного срабатывания на интервал
    double dcr_prob = det_data_.dcr * slot_duration;

    for (unsigned int i = 0; i < photon_times.size(); ++i) {
        double left = time_slots[i] - slot_duration / 2;
        double right = time_slots[i] + slot_duration / 2;
        bool registered = false;

        // проверяем фотоны
        for (double t : photon_times[i]) {
            if (t < left) continue;
            if (t > right) break;

            if (t >= last_reg_time + dead_time) {
                time_registered.push_back(t);
                last_reg_time = t;
                num_registered++;
                registered = true;

                // послеимпульсное срабатывание
                if (rand01(gen) < det_data_.afterpulse_prob) {
                    double after_t = t + det_data_.afterpulse_delay;
                    time_registered.push_back(after_t);
                    num_registered++;
                }
                break;
            }
        }

        // темновой счет (если нет зарегистрированных фотонов)
        if (!registered && rand01(gen) < dcr_prob) {
            double dark_t = time_slots[i] + rand01(gen) * slot_duration;
            time_registered.push_back(dark_t);
            last_reg_time = dark_t;
            num_registered++;
        }
    }

    return num_registered;
}