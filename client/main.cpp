#include <SFML/Graphics.hpp>
#include "CubeState.hpp"
#include "NetworkThread.hpp"
#include <iostream>

int main() {
    SharedCubeState state;
    run_network_thread(&state, "127.0.0.1", 4242);

    sf::RenderWindow window(sf::VideoMode(800, 600), "Jetpack Multiplayer");

    sf::RectangleShape myCube;
    myCube.setSize({50, 50});
    
    sf::Clock clock;
    
    // Wait for player ID assignment (with timeout)
    sf::Time timeout = sf::seconds(5);
    while (state.player_id < 0 && clock.getElapsedTime() < timeout) {
        sf::sleep(sf::milliseconds(100));
    }
    
    if (state.player_id < 0) {
        std::cerr << "Failed to get player ID from server\n";
        return 1;
    }
    
    // Set the cube color based on player ID
    if (state.player_id == 0) {
        myCube.setFillColor(sf::Color::Red);
        window.setTitle("Jetpack Multiplayer - Red Player");
    } else {
        myCube.setFillColor(sf::Color::Blue);
        window.setTitle("Jetpack Multiplayer - Blue Player");
    }
    
    std::cout << "Starting game as player " << state.player_id << std::endl;

    while (window.isOpen()) {
        sf::Event e;
        while (window.pollEvent(e)) {
            if (e.type == sf::Event::Closed)
                window.close();
        }
        
        {
            std::lock_guard<std::mutex> lock(state.mutex);
            if (state.player_id >= 0 && state.player_id < 2) {
                auto& me = state.players[state.player_id];

                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) me.x -= 5;
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) me.x += 5;
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) me.y -= 5;
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) me.y += 5;
                
                // Keep within bounds
                me.x = std::max(0.f, std::min(me.x, 750.f));
                me.y = std::max(0.f, std::min(me.y, 550.f));
            }
        }

        window.clear();
        {
            std::lock_guard<std::mutex> lock(state.mutex);
            // Only render the current player's cube
            myCube.setPosition(state.players[state.player_id].x, state.players[state.player_id].y);
            window.draw(myCube);
        }
        window.display();
        
        // Cap framerate
        sf::sleep(sf::milliseconds(16)); // ~60 FPS
    }

    return 0;
}