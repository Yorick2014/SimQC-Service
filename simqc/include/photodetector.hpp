#pragma once
#include <vector>
#include <algorithm>
#include <random>
#include "simulation_params.hpp"
#include "beam_splitter.hpp"

class IPhotodetector {
public:
    virtual ~IPhotodetector() = default;

    virtual bool detect(const Pulse& p) = 0;

    virtual void generate_photon_timelabels(unsigned int num_pulses,
                                            const std::vector<unsigned int>& num_photons,
                                            std::vector<std::vector<double>>& photon_times,
                                            const std::vector<double>& time_slots,
                                            double time_window) = 0;

    virtual void generate_time_slots(unsigned int num_pulses,
                                     double rep_rate_hz,
                                     std::vector<double>& time_slots) = 0;

    virtual int register_pulses(std::vector<std::vector<double>>& photon_times,
                                std::vector<double>& time_slots,
                                std::vector<double>& time_registered) = 0;
};

class PhotodetectorFactory {
public:
    static std::unique_ptr<IPhotodetector> create(const std::string& type,
                                                 const PhotodetectorData& det_data);
};

class SPAD : public IPhotodetector {
private:
    PhotodetectorData det_data_;

    static double generate_random_0_to_1();

public:
    explicit SPAD(const PhotodetectorData& data)
        : det_data_(data) {}

    bool detect(const Pulse& p) override; 

    void generate_photon_timelabels(unsigned int num_pulses,
                                    const std::vector<unsigned int>& num_photons,
                                    std::vector<std::vector<double>>& photon_times,
                                    const std::vector<double>& time_slots,
                                    double time_window) override;

    void generate_time_slots(unsigned int num_pulses,
                             double rep_rate_hz,
                             std::vector<double>& time_slots) override;

    int register_pulses(std::vector<std::vector<double>>& photon_times,
                        std::vector<double>& time_slots,
                        std::vector<double>& time_registered) override;
};

// class DualPhotodetectorSystem {
// private:
//     std::unique_ptr<IBeamSplitter> splitter;
//     std::unique_ptr<IPhotodetector> detector_H;
//     std::unique_ptr<IPhotodetector> detector_V;

// public:
//     DualPhotodetectorSystem(std::unique_ptr<IBeamSplitter> splitter_,
//                             std::unique_ptr<IPhotodetector> detH,
//                             std::unique_ptr<IPhotodetector> detV)
//         : splitter(std::move(splitter_)),
//           detector_H(std::move(detH)),
//           detector_V(std::move(detV)) {}

//     void process(const std::vector<Pulse>& pulses,
//                  std::vector<std::vector<double>>& times_H,
//                  std::vector<std::vector<double>>& times_V,
//                  std::vector<double>& time_slots,
//                  double slot_duration) {

//         auto [pulses_H, pulses_V] = splitter->split(pulses);

//         detector_H->generate_photon_timelabels(pulses_H.size(),
//                                                extract_num_photons(pulses_H),
//                                                times_H,
//                                                time_slots,
//                                                slot_duration);

//         detector_V->generate_photon_timelabels(pulses_V.size(),
//                                                extract_num_photons(pulses_V),
//                                                times_V,
//                                                time_slots,
//                                                slot_duration);
//     }

// private:
//     // Вспомогательная функция — извлекает количество фотонов из импульсов
//     static std::vector<unsigned int> extract_num_photons(const std::vector<Pulse>& pulses) {
//         std::vector<unsigned int> photons;
//         photons.reserve(pulses.size());
//         for (const auto& p : pulses)
//             photons.push_back(p.count_photons);
//         return photons;
//     }
// };