#pragma once

#include <SFML/Graphics.hpp>
#include <iostream>
#include <unordered_map>
#include <mutex>
#include <algorithm>

#include "../CubeState.hpp"
#include "../NetworkThread.hpp"

class Game {
private:
    SharedCubeState* state;
    sf::RenderWindow window;
    sf::RectangleShape cubes[2];

    //texturas del joc
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

public:
    Game(SharedCubeState* state);
    void run();
    
private:
    void loadBackground();
    void updateBackground(float dt);
    void handleEvents();
    void update(float dt);
    void render();
};