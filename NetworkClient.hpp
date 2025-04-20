#pragma once
#include "CubeState.hpp"
#include <thread>
#include <string>

class NetworkClient {
private:
    SharedCubeState* state;
    int sock;
    std::thread network_thread;
    bool running;
    
    void send_input_to_server();
    void process_server_messages();
    void network_loop();

public:
    NetworkClient(SharedCubeState* state);
    ~NetworkClient();
    
    bool connect(const char* ip, int port);
    void disconnect();
    bool is_connected() const;
};