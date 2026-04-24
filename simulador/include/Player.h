#ifndef PLAYER_H
#define PLAYER_H

#include <SFML/Graphics.hpp>
#include "Interfaces.h"
#include <memory>

class Player {
public:
    Player(int id, float radius, sf::Color color, std::unique_ptr<PlayerAgent> agent);
    void update(const GameState& state, float dt);
    void draw(sf::RenderWindow& window, const sf::Font& font, bool isSelected, sf::Vector2f pixPos, float scale);
    void setPosition(float x, float y);
    sf::Vector2f getPosition() const;
    sf::Vector2f getVelocity() const;
    void setVelocity(sf::Vector2f v);
    float getRadius() const;
    Action getLastAction() const;
    int getId() const { return id; }

private:
    int id;
    float radius; // Raio original em metros
    sf::Vector2f pos; // Posição oficial em metros
    sf::CircleShape shape;
    sf::Vector2f velocity;
    std::unique_ptr<PlayerAgent> agent;
    Action lastAction;
    float maxSpeed = 200.0f;
};

#endif // PLAYER_H
