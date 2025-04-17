#include <SFML/Graphics.hpp>
#include <iostream>
#include <unordered_map>

#include "../CubeState.hpp"
#include "../Server/Server.hpp"
#include "../NetworkThread.hpp"

int main() {
    SharedCubeState state;
    run_network_thread(&state, "127.0.0.1", 4242);

    sf::RenderWindow window(sf::VideoMode(800, 600), "Jetpack Multiplayer");

    sf::RectangleShape cubes[2];
    for (std::size_t i = 0; i < 2; i++) {
        cubes[i].setSize({50, 50});
        cubes[i].setFillColor(i % 2 == 0 ? sf::Color::Red : sf::Color::Green);
    }

    // Physics constants
    const float gravity = 0.8f;        // Acceleration due to gravity
    const float jump_power = 5.0f;
    const float max_fall_speed = 10.0f;
    const float cube_size = 50.0f;
    const float window_width = 800.0f;
    const float window_height = 600.0f;

    // Store vertical velocities for each player
    float player_velocities[2] = {0.0f, 0.0f};

    sf::Clock clock;

    while (window.isOpen()) {
        float dt = clock.restart().asSeconds();
        
        // Limit dt to prevent physics issues if game freezes momentarily
        if (dt > 0.1f) dt = 0.1f;

        sf::Event e;
        while (window.pollEvent(e)) {
            if (e.type == sf::Event::Closed) {
                window.close();
            }
        }

        if (window.hasFocus()) {
            std::lock_guard<std::mutex> lock(state.mutex);
            if (state.player_id != -1) {
                auto& me = state.players[state.player_id];
                float& velocity = player_velocities[state.player_id];
                
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) {
                    velocity -= jump_power * dt;
                }
                
                velocity += gravity * dt;
                
                // Cap fall speed
                if (velocity > max_fall_speed) {
                    velocity = max_fall_speed;
                }
                
                // Update position
                me.y += velocity;
                
                // Optional: Add horizontal movement
                const float h_speed = 200.0f;
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) {
                    me.x -= h_speed * dt;
                }
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) {
                    me.x += h_speed * dt;
                }
                
                // Handle collisions with screen edges
                if (me.y > window_height - cube_size) {
                    me.y = window_height - cube_size;
                    velocity = 0; // Stop at floor
                }
                if (me.y < 0) {
                    me.y = 0;
                    velocity = 0; // Stop at ceiling
                }
                
                // Keep player within horizontal bounds
                me.x = std::clamp(me.x, 0.0f, window_width - cube_size);
                
                // Debug output (uncomment if needed)
                // std::cout << "Position: " << me.x << ", " << me.y << " Velocity: " << velocity << std::endl;
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