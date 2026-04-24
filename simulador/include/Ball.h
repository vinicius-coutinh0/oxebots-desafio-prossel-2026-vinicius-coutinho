#ifndef BALL_H
#define BALL_H

#include <SFML/Graphics.hpp>

class Ball {
public:
    Ball(float radius);
    void update(float dt);
    void draw(sf::RenderWindow& window);
    void setPosition(float x, float y);
    sf::Vector2f getPosition() const;
    sf::Vector2f getVelocity() const;
    void setVelocity(sf::Vector2f v);
    float getRadius() const;

private:
    sf::CircleShape shape;
    sf::Vector2f velocity;
    float friction = 0.98f; // Desaceleração por frame
};

#endif // BALL_H
