#pragma once
#include <httplib.h>
#include <string>

namespace api {
namespace v1 {

/**
 * @brief Регистрирует все маршруты для API версии 1.
 *
 * Здесь описываются HTTP endpoints, например:
 *  - POST /api/v1/config   — загрузка конфигурации
 *  - POST /api/v1/start    — запуск моделирования
 *  - POST /api/v1/stop     — остановка моделирования
 *  - GET  /api/v1/status   — получение состояния моделирования
 */
void register_routes(httplib::Server& svr);

/**
 * @brief Вспомогательные обработчики (контроллеры) маршрутов.
 *
 * Каждый обработчик реализуется в simulation_controller.cpp
 */
namespace handlers {

/// Загружает конфигурацию модели
void load_config(const httplib::Request& req, httplib::Response& res);

/// Запускает моделирование (асинхронно)
void start_simulation(const httplib::Request& req, httplib::Response& res);

/// Прерывает текущее моделирование
void stop_simulation(const httplib::Request& req, httplib::Response& res);

/// Проверяет статус моделирования
void get_status(const httplib::Request& req, httplib::Response& res);

} // namespace handlers

} // namespace v1
} // namespace api
