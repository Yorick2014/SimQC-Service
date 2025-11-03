#pragma once
#include <httplib.h>

/**
 * @brief ApiRouter — класс для регистрации всех версий API.
 * 
 * Здесь регистрируются маршруты разных версий (например, v1, v2 и т.д.).
 */
class ApiRouter {
public:
    /**
     * @brief Регистрирует все маршруты API на сервере httplib.
     * 
     * @param svr — экземпляр httplib::Server, на который навешиваются маршруты.
     */
    static void register_all(httplib::Server& svr);
};
