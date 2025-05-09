#include "Game.hpp"

Game::Game(SharedCubeState* state) : state(state) {
    window.create(sf::VideoMode(800, 600), "Jetpack Multiplayer");
    coinTexture.loadFromFile("img/coin.png");
    for (std::size_t i = 0; i < 2; i++) {
        cubes[i].setSize({50, 50});
        cubes[i].setFillColor(i % 2 == 0 ? sf::Color::Red : sf::Color::Green);
    }

    coinSprite.setTexture(coinTexture);
    // Scale coin to appropriate size
    coinSprite.setScale(0.5f, 0.5f);
    
    // For fallback shape
    coinShape.setRadius(15.f);
    coinShape.setFillColor(sf::Color::Yellow);
    coinShape.setOrigin(15.f, 15.f);
    
    loadBackground();
}

void Game::loadBackground() {
    if (!backgroundTexture.loadFromFile("img/Game_Background_190-518759431.png")) {
        std::cerr << "Failed to load background texture!" << std::endl;
        return;
    }
    
    backgroundSprite1.setTexture(backgroundTexture);
    backgroundSprite2.setTexture(backgroundTexture);
    
    float scaleY = window_height / backgroundTexture.getSize().y;
    backgroundSprite1.setScale(scaleY, scaleY);
    backgroundSprite2.setScale(scaleY, scaleY);
    
    backgroundSprite1.setPosition(0, 0);
    backgroundSprite2.setPosition(backgroundTexture.getSize().x * scaleY, 0);
}

void Game::updateBackground(float dt) {
    backgroundOffset += backgroundScrollSpeed * dt;
    float scaledWidth = backgroundTexture.getSize().x * backgroundSprite1.getScale().x;
    
    if (backgroundOffset >= scaledWidth) {
        backgroundOffset -= scaledWidth;
    }
    
    backgroundSprite1.setPosition(-backgroundOffset, 0);
    backgroundSprite2.setPosition(scaledWidth - backgroundOffset, 0);
}

void Game::run() {
    while (window.isOpen()) {
        float dt = clock.restart().asSeconds();
        
        // Limite de las fisicas
        if (dt > 0.1f) dt = 0.1f;
        
        handleEvents();
        updateBackground(dt);
        updateInputs();
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

void Game::updateInputs() {
    if (window.hasFocus() && state->player_id != -1) {
        std::lock_guard<std::mutex> lock(state->mutex);
                state->input.jump = sf::Keyboard::isKeyPressed(sf::Keyboard::Space);

    }
}

void Game::render() {
    window.clear();
    
    window.draw(backgroundSprite1);
    window.draw(backgroundSprite2);
    
    // Dibujar los jugadores
    {
        std::lock_guard<std::mutex> lock(state->mutex);
        for (int i = 0; i < 2; ++i) {
            if (state->players[i].active) {
                cubes[i].setPosition(state->players[i].x, state->players[i].y);
                window.draw(cubes[i]);
            }
        }
    }
    // for (size_t i = 0; i < state->coins.size(); i++) {
    //         if (state->coins[i].active) {
    //             if (useCoinShape) {
    //                 coinShape.setPosition(state->coins[i].x, state->coins[i].y);
    //                 window.draw(coinShape);
    //             } else {
    //                 coinSprite.setPosition(state->coins[i].x, state->coins[i].y);
    //                 window.draw(coinSprite);
    //             }
    //         }
    //     }
    
    window.display();
}