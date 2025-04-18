#include "Game.hpp"

Game::Game(SharedCubeState* state) : state(state) {
    window.create(sf::VideoMode(800, 600), "Jetpack Multiplayer");
    
    for (std::size_t i = 0; i < 2; i++) {
        cubes[i].setSize({50, 50});
        cubes[i].setFillColor(i % 2 == 0 ? sf::Color::Red : sf::Color::Green);
    }

    state->players[0].x = 500.0f;
    state->players[0].y = 300.0f;

    state->players[1].x = 600.0f;
    state->players[1].y = 300.0f;
    
    loadBackground();
}

void Game::loadBackground() {
    if (!backgroundTexture.loadFromFile("img/Game_Background_190-518759431.png")) {
        std::cerr << "Failed to load background texture!" << std::endl;
        return;
    }
    
    // Set up the repeating background sprites
    backgroundSprite1.setTexture(backgroundTexture);
    backgroundSprite2.setTexture(backgroundTexture);
    
    // Scale background to fit window height if needed
    float scaleY = window_height / backgroundTexture.getSize().y;
    backgroundSprite1.setScale(scaleY, scaleY);
    backgroundSprite2.setScale(scaleY, scaleY);
    
    // Initially position sprites
    backgroundSprite1.setPosition(0, 0);
    backgroundSprite2.setPosition(backgroundTexture.getSize().x * scaleY, 0);
}

void Game::updateBackground(float dt) {
    // Update background offset
    backgroundOffset += backgroundScrollSpeed * dt;
    
    // Get the effective width of the scaled background
    float scaledWidth = backgroundTexture.getSize().x * backgroundSprite1.getScale().x;
    
    // Reset the offset when it exceeds the width of one background image
    if (backgroundOffset >= scaledWidth) {
        backgroundOffset -= scaledWidth;
    }
    
    // Position the two background sprites for continuous scrolling
    backgroundSprite1.setPosition(-backgroundOffset, 0);
    backgroundSprite2.setPosition(scaledWidth - backgroundOffset, 0);
}

void Game::run() {
    while (window.isOpen()) {
        float dt = clock.restart().asSeconds();
        
        // Limit dt to prevent physics issues if game freezes momentarily
        if (dt > 0.1f) dt = 0.1f;
        
        handleEvents();
        updateBackground(dt);
        update(dt);
        render();
    }
}

void Game::handleEvents() {
    sf::Event e;
    while (window.pollEvent(e)) {
        if (e.type == sf::Event::Closed) {
            window.close();
        }
    }
}

void Game::update(float dt) {
    if (window.hasFocus()) {
        std::lock_guard<std::mutex> lock(state->mutex);
        if (state->player_id != -1) {
            auto& me = state->players[state->player_id];
            float& velocity = player_velocities[state->player_id];
            
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
        }
    }
}

void Game::render() {
    window.clear();
    
    // Draw background first
    window.draw(backgroundSprite1);
    window.draw(backgroundSprite2);
    
    // Draw players
    {
        std::lock_guard<std::mutex> lock(state->mutex);
        for (int i = 0; i < 2; ++i) {
            cubes[i].setPosition(state->players[i].x, state->players[i].y);
            window.draw(cubes[i]);
        }
    }
    
    window.display();
}