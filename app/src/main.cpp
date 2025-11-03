#include "api/api_router.hpp"
#include <httplib.h>
#include <iostream>

void start_server() {
    httplib::Server svr;
    ApiRouter::register_all(svr);

    std::cout << "Server started on port 5555" << std::endl;
    svr.listen("0.0.0.0", 5555);
}

int main() {
    std::cout << "Start server" << std::endl;
    start_server();
    return 0;
}