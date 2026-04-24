#ifndef SIMULATOR_H
#define SIMULATOR_H

#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>
#include "Ball.h"
#include "Player.h"

class Simulator {
public:
    Simulator(unsigned int width, unsigned int height);
    void run();

private:
    void processEvents();
    void update(float dt);
    void render();
    void handleCollisions();
    void resetPositions(bool resetBall = true);
    GameState getGameState(size_t playerIndex, bool teamA);

    sf::RenderWindow window;
    sf::Font font;
    sf::Text scoreText;
    sf::Text timerText;

    int scoreA = 0;
    int scoreB = 0;
    float gameTime = 0.0f;
    float lastLogTime = 0.0f;

    Ball ball;
    std::vector<std::unique_ptr<Player>> teamA;
    std::vector<std::unique_ptr<Player>> teamB;
    
    sf::RectangleShape field;
    sf::RectangleShape goalA;
    sf::RectangleShape goalB;
    sf::RectangleShape areaA;
    sf::RectangleShape areaB;
    sf::CircleShape centerCircle;
    sf::RectangleShape centerLine;

    float width, height;
};

#endif // SIMULATOR_H
