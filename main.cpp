#include "Server.hpp"
#include <iostream>

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: ./jetpack_server -p <port> -m <map>\n";
        return 84;
    }

    int port = 0;
    std::string map_path;

    for (int i = 1; i < argc; i++) {
        if (std::string(argv[i]) == "-p" && i + 1 < argc) {
            port = std::stoi(argv[++i]);
        } else if (std::string(argv[i]) == "-m" && i + 1 < argc) {
            map_path = argv[++i];
        }
    }

    if (port == 0 || map_path.empty()) {
        std::cerr << "Invalid arguments\n";
        return 84;
    }

    Server server(port, map_path);
    return server.run() ? 0 : 84;
}
