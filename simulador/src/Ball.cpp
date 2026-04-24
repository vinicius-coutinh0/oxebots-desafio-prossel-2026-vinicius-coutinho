#include "Ball.h"
#include <cmath>

Ball::Ball(float radius) : velocity(0.0f, 0.0f) {
    shape.setRadius(radius);
    shape.setOrigin(radius, radius);
    shape.setFillColor(sf::Color::White);
    shape.setOutlineThickness(1.0f);
    shape.setOutlineColor(sf::Color::Black);
}

void Ball::update(float dt) {
    shape.move(velocity * dt);
    velocity *= friction; // Atrito simples
}

void Ball::draw(sf::RenderWindow& window) {
    window.draw(shape);
}

void Ball::setPosition(float x, float y) {
    shape.setPosition(x, y);
}

sf::Vector2f Ball::getPosition() const {
    return shape.getPosition();
}

sf::Vector2f Ball::getVelocity() const {
    return velocity;
}

void Ball::setVelocity(sf::Vector2f v) {
    velocity = v;
}

float Ball::getRadius() const {
    return shape.getRadius();
}
