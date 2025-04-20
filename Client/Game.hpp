#pragma once

#include <SFML/Graphics.hpp>
#include <iostream>
#include <unordered_map>
#include <mutex>
#include <algorithm>

#include "../CubeState.hpp"
#include "../NetworkClient.hpp"

class Game {
private:
    SharedCubeState *state;
    sf::RenderWindow window;
    sf::RectangleShape cubes[2];

    sf::Texture backgroundTexture;
    sf::Sprite backgroundSprite1;
    sf::Sprite backgroundSprite2;
    float backgroundScrollSpeed = 50.0f;
    float backgroundOffset = 0.0f;
    
    const float gravity = 0.8f;
    const float jump_power = 5.0f;
    const float max_fall_speed = 10.0f;
    const float cube_size = 50.0f;
    const float window_width = 800.0f;
    const float window_height = 600.0f;
    
    float player_velocities[2] = {0.0f, 0.0f};
    
    sf::Clock clock;

    void loadBackground();
    void updateBackground(float dt);
    void handleEvents();
    void update(float dt);
    void render();
    void updateInputs();

    sf::Texture coinTexture;
    sf::Sprite coinSprite;
    sf::CircleShape coinShape;
    bool useCoinShape = false;
public:
    Game(SharedCubeState *state);
    void run();
    
};