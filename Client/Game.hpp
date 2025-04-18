#pragma once

#include <SFML/Graphics.hpp>
#include <iostream>
#include <unordered_map>
#include <mutex>
#include <algorithm>
#include <vector>
#include <random>

#include "../CubeState.hpp"
#include "../NetworkThread.hpp"

// Game element classes
class Coin {
public:
    sf::CircleShape shape;  // Fallback shape if texture fails
    sf::Sprite sprite;      // Sprite for texture
    bool active = true;
    bool useSprite = false;
    
    Coin(float x, float y) {
        // Setup basic shape
        shape.setRadius(15.0f);
        shape.setFillColor(sf::Color::Yellow);
        shape.setOrigin(15.0f, 15.0f);
        shape.setPosition(x, y);
        
        // Sprite position will be set when texture is loaded
        sprite.setPosition(x, y);
    }
    
    void setTexture(const sf::Texture& texture) {
        sprite.setTexture(texture);
        // Center the sprite
        sprite.setOrigin(texture.getSize().x / 2.0f, texture.getSize().y / 2.0f);
        sprite.setPosition(shape.getPosition());

        sprite.setScale(0.1f, 0.1f);
        useSprite = true;
    }
    
    void setPosition(float x, float y) {
        shape.setPosition(x, y);
        sprite.setPosition(x, y);
    }
    
    sf::Vector2f getPosition() const {
        return shape.getPosition();
    }
    
    sf::FloatRect getGlobalBounds() const {
        return useSprite ? sprite.getGlobalBounds() : shape.getGlobalBounds();
    }
};

class Obstacle {
public:
    sf::RectangleShape shape;  // Fallback shape if texture fails
    sf::Sprite sprite;         // Sprite for texture
    float speed = 200.0f;      // Horizontal movement speed
    bool useSprite = false;
    
    Obstacle(float x, float y, float width, float height) {
        // Setup basic shape
        shape.setSize(sf::Vector2f(width, height));
        shape.setFillColor(sf::Color(150, 75, 0));  // Brown color
        shape.setPosition(x, y);
        
        // Sprite position will be set when texture is loaded
        sprite.setPosition(x, y);
    }
    
    void setTexture(const sf::Texture& texture) {
        sprite.setTexture(texture);
        // Scale to match the shape's size
        float scaleX = shape.getSize().x / texture.getSize().x;
        float scaleY = shape.getSize().y / texture.getSize().y;
        sprite.setScale(scaleX, scaleY);
        sprite.setPosition(shape.getPosition());
        useSprite = true;
    }
    
    void setPosition(float x, float y) {
        shape.setPosition(x, y);
        sprite.setPosition(x, y);
    }
    
    sf::Vector2f getPosition() const {
        return shape.getPosition();
    }
    
    sf::FloatRect getGlobalBounds() const {
        return useSprite ? sprite.getGlobalBounds() : shape.getGlobalBounds();
    }
};

class Game {
private:
    SharedCubeState* state;
    sf::RenderWindow window;
    sf::RectangleShape cubes[2];

    // Game state
    bool gameRunning = true;
    
    // Game elements
    std::vector<Coin> coins;
    std::vector<Obstacle> obstacles;
    float spawnTimer = 0.0f;
    const float obstacleSpawnInterval = 2.0f;
    int score = 0;
    
    // Textures
    sf::Texture backgroundTexture;
    sf::Texture coinTexture;
    sf::Texture obstacleTexture;
    
    // Background elements
    sf::Sprite backgroundSprite1;
    sf::Sprite backgroundSprite2;
    float backgroundScrollSpeed = 50.0f;
    float backgroundOffset = 0.0f;
    
    // Physics constants
    const float gravity = 0.8f;
    const float jump_power = 5.0f;
    const float max_fall_speed = 10.0f;
    const float cube_size = 50.0f;
    const float window_width = 800.0f;
    const float window_height = 600.0f;
    
    // Store vertical velocities for each player
    float player_velocities[2] = {0.0f, 0.0f};
    
    sf::Clock clock;
    std::mt19937 rng;  // Random number generator

public:
    Game(SharedCubeState* state);
    void run();
    
private:
    void loadTextures();
    void loadBackground();
    void updateBackground(float dt);
    void handleEvents();
    void update(float dt);
    void updateCoins(float dt);
    void updateObstacles(float dt);
    void spawnObstacle();
    void spawnCoins();
    void checkCollisions();
    void render();
    void gameOver();
    float getRandomY();
};