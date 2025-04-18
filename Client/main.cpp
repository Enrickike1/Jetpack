#include <SFML/Graphics.hpp>
#include "Game.hpp"
#include "../CubeState.hpp"
#include "../Server/Server.hpp"
#include "../NetworkThread.hpp"

int main() {
    SharedCubeState state;
    run_network_thread(&state, "127.0.0.1", 4242);

    Game game(&state);
    game.run();

    return 0;
}