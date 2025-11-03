#include "api/api_router.hpp"
#include "api/v1/simulation_routes.hpp"
#include <httplib.h>

void ApiRouter::register_all(httplib::Server& svr) {
    api::v1::register_routes(svr);
}
