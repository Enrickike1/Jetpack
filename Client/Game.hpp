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
    sf::CircleShape shape;
    bool active = true;
    
    Coin(float x, float y) {
        shape.setRadius(15.0f);
        shape.setFillColor(sf::Color::Yellow);
        shape.setOrigin(15.0f, 15.0f);
        shape.setPosition(x, y);
    }
};

class Obstacle {
public:
    sf::RectangleShape shape;
    float speed = 200.0f;  // Horizontal movement speed
    
    Obstacle(float x, float y, float width, float height) {
        shape.setSize(sf::Vector2f(width, height));
        shape.setFillColor(sf::Color(150, 75, 0));  // Brown color
        shape.setPosition(x, y);
    }
};

class Game {
private:
    SharedCubeState* state;
    sf::RenderWindow window;
    sf::RectangleShape cubes[2];

    // Game elements
    std::vector<Coin> coins;
    std::vector<Obstacle> obstacles;
    float spawnTimer = 0.0f;
    const float obstacleSpawnInterval = 2.0f;
    int score = 0;
    
    // Background elements
    sf::Texture backgroundTexture;
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
    float getRandomY();
};