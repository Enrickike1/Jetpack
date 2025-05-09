#include <SFML/Graphics.hpp>
#include "Game.hpp"
#include "../CubeState.hpp"
#include "../Server/Server.hpp"
#include "../NetworkClient.hpp"

int main() {
    SharedCubeState state;
    
    NetworkClient network(&state);
    if (!network.connect("127.0.0.1", 4242)) {
        std::cerr << "Failed to connect to server. Exiting.\n";
        return 1;
    }

    Game game(&state);
    game.run();
    network.disconnect();

    return 0;
}