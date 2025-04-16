#include <SFML/Graphics.hpp>
#include <iostream>
#include <thread>
#include "CubeState.hpp"
#include "NetworkThread.hpp"

int main() {
    SharedCubeState state;
    run_network_thread(&state, "127.0.0.1", 4242);

    sf::RenderWindow window(sf::VideoMode(800, 600), "Jetpack Multiplayer");

    sf::RectangleShape cubes[2];
    cubes[0].setSize({50, 50});
    cubes[0].setFillColor(sf::Color::Red);
    cubes[1].setSize({50, 50});
    cubes[1].setFillColor(sf::Color::Blue);

    while (window.isOpen()) {
        sf::Event e;
        while (window.pollEvent(e)) {
            if (e.type == sf::Event::Closed)
                window.close();
        }

        // Process movement only if the window is in focus.
        if (window.hasFocus()) {
            std::lock_guard<std::mutex> lock(state.mutex);
            // Ensure that player_id has been set (i.e., not -1).
            if(state.player_id != -1){
                auto& me = state.players[state.player_id];

                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
                    me.x -= 2;
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
                    me.x += 2;
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
                    me.y -= 2;
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
                    me.y += 2;
            }
        }

        window.clear();
        {
            std::lock_guard<std::mutex> lock(state.mutex);
            for (int i = 0; i < 2; ++i) {
                cubes[i].setPosition(state.players[i].x, state.players[i].y);
                window.draw(cubes[i]);
            }
        }
        window.display();
    }

    return 0;
}
