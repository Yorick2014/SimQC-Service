#include "api/v1/simulation_routes.hpp"
#include "api/v1/simulation_controller.hpp"
#include <httplib.h>
#include <nlohmann/json.hpp>
#include <uuid/uuid.h>

using json = nlohmann::json;

namespace api::v1 {

void register_routes(httplib::Server& svr) {
    svr.Post("/api/v1/config", [](const httplib::Request& req, httplib::Response& res) {
        try {
            auto j = json::parse(req.body);
            Common params = j["common"].get<Common>();
            LaserData laser = j["laser"].get<LaserData>();
            QuantumChannelData q_channel_data = j["q_channel"].get<QuantumChannelData>();
            PhotodetectorData ph_data = j["photodetector"].get<PhotodetectorData>();
            controller.load_config(params, laser, q_channel_data, ph_data);
            // res.status = 200;
            res.set_content("Configuration loaded", "text/plain");
        } catch (const std::exception& e) {
            res.status = 400;
            res.set_content(std::string("Invalid config JSON: ") + e.what(), "text/plain");
        }
    });

    svr.Post("/api/v1/start", [&](const httplib::Request& req, httplib::Response& res) {
        if (controller.is_running()) {
            res.status = 409;
            res.set_content(R"({"error":"Already running"})", "application/json");
            return;
        }

        try {
            auto j = json::parse(req.body);
            if (!j.contains("start") || !j["start"].get<bool>()) {
                res.status = 400;
                res.set_content(R"({"error":"Invalid request"})", "application/json");
                return;
            }
        } catch (...) {
            res.status = 400;
            res.set_content(R"({"error":"Bad JSON"})", "application/json");
            return;
        }

        uuid_t uuid;
        uuid_generate(uuid);

        char uuid_str[37];
        uuid_unparse(uuid, uuid_str);

        std::string run_id = uuid_str;

        controller.start(run_id);

        json response;
        response["run_id"] = uuid_str;
        response["status"] = "started";

        res.set_content(response.dump(), "application/json");
    });

    svr.Get(R"(/api/v1/results/(\w+))", [](const httplib::Request& req, httplib::Response& res) {
        std::string run_id = req.matches[1];

        std::string stats_path = "results/bb84_" + run_id + "/stats.csv";
        if (!std::filesystem::exists(stats_path)) {
            res.status = 404;
            res.set_content(R"({"error":"Run not found"})", "application/json");
            return;
        }

        std::ifstream fin(stats_path);
        std::stringstream buffer;
        buffer << fin.rdbuf();
        fin.close();

        res.set_content(buffer.str(), "text/csv");
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
