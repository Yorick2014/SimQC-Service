#include "api/v1/simulation_routes.hpp"
#include "api/v1/simulation_controller.hpp"
#include <httplib.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace api::v1 {

void register_routes(httplib::Server& svr) {
    svr.Post("/api/v1/config", [](const httplib::Request& req, httplib::Response& res) {
        try {
            auto j = json::parse(req.body);
            Common params = j["common"].get<Common>();
            LaserData laser = j["laser"].get<LaserData>();
            controller.load_config(params, laser);
            // res.status = 200;
            res.set_content("Configuration loaded", "text/plain");
        } catch (const std::exception& e) {
            res.status = 400;
            res.set_content(std::string("Invalid config JSON: ") + e.what(), "text/plain");
        }
    });

    svr.Post("/api/v1/start", [](const httplib::Request& req, httplib::Response& res) {
        std::cout << "/start hit" << std::endl;
        
        if (controller.is_running()) {
            res.status = 409;
            res.set_content("Already running", "text/plain");
            return;
        }

        try
        {
            auto j = json::parse(req.body);
            if (j["start"]) controller.start();
            
        }
        catch(const std::exception& e)
        {
            std::cerr << e.what() << '\n';
        }

        res.set_content("Sim running", "text/plain");
    });

    svr.Post("/api/v1/stop", [](const httplib::Request&, httplib::Response& res) {
        controller.stop();
        // res.status = 200;
        json j = {
            {"running", controller.is_running()}
        };
        res.set_content(j.dump(2), "application/json");
    });

    svr.Get("/api/v1/status", [](const httplib::Request&, httplib::Response& res) {
        json j = {
            {"running", controller.is_running()},
            {"stop_requested", controller.stop_requested()}
        };
        // res.status = 200;
        res.set_content(j.dump(2), "application/json");
    });
}
} // namespace api::v1
