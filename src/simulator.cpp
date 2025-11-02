#include "simulator.hpp"

nlohmann::json Simulator::run(const nlohmann::json& params) {
    // Пример: читаем параметры
    int photons = params.value("photons", 100);
    double loss = params.value("loss", 0.1);

    // Здесь — запуск твоей симуляции
    // (можно подключить существующие классы из проекта)

    nlohmann::json result;
    result["status"] = "ok";
    result["used_photons"] = photons;
    result["loss"] = loss;
    return result;
}
