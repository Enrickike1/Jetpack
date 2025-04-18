#include "Game.hpp"
#include <ctime>

Game::Game(SharedCubeState* state) : state(state) {
    window.create(sf::VideoMode(800, 600), "Jetpack Multiplayer");
    
    for (std::size_t i = 0; i < 2; i++) {
        cubes[i].setSize({50, 50});
        cubes[i].setFillColor(i % 2 == 0 ? sf::Color::Red : sf::Color::Green);
    }

    state->players[0].x = 150.0f;  // Position players more to the left
    state->players[0].y = 300.0f;

    state->players[1].x = 250.0f;
    state->players[1].y = 300.0f;
    
    // Initialize random number generator
    rng.seed(static_cast<unsigned int>(std::time(nullptr)));
    
    loadTextures();
    loadBackground();
    spawnCoins();  // Create initial coins
}

void Game::loadTextures() {
    // Load background texture in loadBackground() method
    
    // Load coin texture
    if (!coinTexture.loadFromFile("img/coin.png")) {
        std::cerr << "Failed to load coin texture! Using fallback shape." << std::endl;
    }
    
    // Load obstacle texture
    if (!obstacleTexture.loadFromFile("img/obstacle.png")) {
        std::cerr << "Failed to load obstacle texture! Using fallback shape." << std::endl;
    }
}

float Game::getRandomY() {
    std::uniform_real_distribution<float> dist(50.0f, window_height - 100.0f);
    return dist(rng);
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

void Game::spawnCoins() {
    // Create some initial coins at different positions
    for (int i = 0; i < 10; i++) {
        float x = window_width + i * 150.0f;  // Spread coins out horizontally
        float y = getRandomY();
        coins.emplace_back(x, y);
        
        // Apply texture to the coin if available
        if (coinTexture.getSize().x > 0) {
            coins.back().setTexture(coinTexture);
        }
    }
}

void Game::spawnObstacle() {
    // Vary obstacle height and vertical position
    float height = std::uniform_real_distribution<float>(100.0f, 200.0f)(rng);
    float y = std::uniform_real_distribution<float>(0.0f, window_height - height)(rng);
    
    // Create a new obstacle at the right edge of the screen
    obstacles.emplace_back(window_width, y, 50.0f, height);
    
    // Apply texture to the obstacle if available
    if (obstacleTexture.getSize().x > 0) {
        obstacles.back().setTexture(obstacleTexture);
    }
}

void Game::updateCoins(float dt) {
    if (!gameRunning) return;
    
    // Move all coins to the left
    for (auto& coin : coins) {
        sf::Vector2f pos = coin.getPosition();
        pos.x -= backgroundScrollSpeed * dt;
        coin.setPosition(pos.x, pos.y);
    }
    
    // Remove coins that have moved off the left edge of the screen
    coins.erase(
        std::remove_if(coins.begin(), coins.end(), 
            [](const Coin& coin) { 
                return !coin.active || coin.getPosition().x < -30.0f; 
            }),
        coins.end()
    );
    
    // Add new coins if needed
    if (coins.size() < 10) {
        float lastX = window_width;
        if (!coins.empty()) {
            lastX = 0;
            for (const auto& coin : coins) {
                lastX = std::max(lastX, coin.getPosition().x);
            }
        }
        
        float x = std::max(lastX + 150.0f, window_width);
        float y = getRandomY();
        
        coins.emplace_back(x, y);
        
        // Apply texture to the coin if available
        if (coinTexture.getSize().x > 0) {
            coins.back().setTexture(coinTexture);
        }
    }
}

void Game::updateObstacles(float dt) {
    if (!gameRunning) return;
    
    // Update spawn timer
    spawnTimer += dt;
    if (spawnTimer >= obstacleSpawnInterval) {
        spawnTimer = 0;
        spawnObstacle();
    }
    
    // Move all obstacles to the left
    for (auto& obstacle : obstacles) {
        sf::Vector2f pos = obstacle.getPosition();
        pos.x -= obstacle.speed * dt;
        obstacle.setPosition(pos.x, pos.y);
    }
    
    // Remove obstacles that have moved off the left edge of the screen
    obstacles.erase(
        std::remove_if(obstacles.begin(), obstacles.end(),
            [](const Obstacle& obstacle) {
                return obstacle.getPosition().x + obstacle.shape.getSize().x < 0;
            }),
        obstacles.end()
    );
}

void Game::gameOver() {
    gameRunning = false;
    std::cout << "Game Over! Final score: " << score << std::endl;
    std::cout << "Press Enter to exit..." << std::endl;
    
    // Wait for Enter key press before closing
    bool waitForEnter = true;
    while (waitForEnter && window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
                return;
            }
            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Enter) {
                waitForEnter = false;
                window.close();
                return;
            }
        }
        
        // Keep rendering the current scene
        render();
    }
}

void Game::checkCollisions() {
    if (!gameRunning) return;
    
    std::lock_guard<std::mutex> lock(state->mutex);
    if (state->player_id == -1) return;
    
    auto& me = state->players[state->player_id];
    
    // Create a rectangle representing the player's hitbox
    sf::FloatRect playerBounds(me.x, me.y, cube_size, cube_size);
    
    // Check collision with coins
    for (auto& coin : coins) {
        if (!coin.active) continue;
        
        sf::FloatRect coinBounds = coin.getGlobalBounds();
        if (playerBounds.intersects(coinBounds)) {
            // Collect the coin
            coin.active = false;
            score += 10;
            std::cout << "Score: " << score << std::endl;
        }
    }
    
    // Check collision with obstacles
    for (const auto& obstacle : obstacles) {
        sf::FloatRect obstacleBounds = obstacle.getGlobalBounds();
        if (playerBounds.intersects(obstacleBounds)) {
            // End the game
            gameOver();
            return;
        }
    }
}

void Game::run() {
    while (window.isOpen() && gameRunning) {
        float dt = clock.restart().asSeconds();
        
        // Limit dt to prevent physics issues if game freezes momentarily
        if (dt > 0.1f) dt = 0.1f;
        
        handleEvents();
        updateBackground(dt);
        updateCoins(dt);
        updateObstacles(dt);
        update(dt);
        checkCollisions();
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
    if (!gameRunning) return;
    
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
    
    // Draw coins
    for (const auto& coin : coins) {
        if (coin.active) {
            if (coin.useSprite) {
                window.draw(coin.sprite);
            } else {
                window.draw(coin.shape);
            }
        }
    }
    
    // Draw obstacles
    for (const auto& obstacle : obstacles) {
        if (obstacle.useSprite) {
            window.draw(obstacle.sprite);
        } else {
            window.draw(obstacle.shape);
        }
    }
    
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