#include "Player.h"
#include <cmath>

Player::Player(int id, float radius, sf::Color color, std::unique_ptr<PlayerAgent> agent) 
    : id(id), radius(radius), pos(0,0), velocity(0.0f, 0.0f), agent(std::move(agent)) {
    shape.setRadius(radius);
    shape.setOrigin(radius, radius);
    shape.setFillColor(color);
    shape.setOutlineColor(sf::Color::Black);
}

void Player::update(const GameState& state, float dt) {
    if (agent) {
        lastAction = agent->think(state);
        sf::Vector2f dir(lastAction.moveDirectionX, lastAction.moveDirectionY);
        float length = std::sqrt(dir.x * dir.x + dir.y * dir.y);
        
        if (length > 0.0001f) {
            if (length > 1.0f) dir /= length;
            velocity = dir * 0.5f; // Velocidade máxima de 0.5 m/s
        } else {
            velocity = sf::Vector2f(0, 0);
        }
        
        pos += velocity * dt; // Atualiza a posição em metros
    }
}

void Player::draw(sf::RenderWindow& window, const sf::Font& font, bool isSelected, sf::Vector2f pixPos, float scale) {
    float screenRadius = radius * scale;
    shape.setRadius(screenRadius);
    shape.setOrigin(screenRadius, screenRadius);
    shape.setPosition(pixPos); // Define a posição apenas para o desenho
    
    if (isSelected) {
        shape.setOutlineThickness(4.0f);
        shape.setOutlineColor(sf::Color::Yellow);
    } else {
        shape.setOutlineThickness(2.0f);
        shape.setOutlineColor(sf::Color::Black);
    }
    window.draw(shape);

    sf::Text text;
    text.setFont(font);
    text.setString(std::to_string(id));
    text.setCharacterSize(16);
    text.setFillColor(sf::Color::White);
    text.setStyle(sf::Text::Bold);
    
    sf::FloatRect textBounds = text.getLocalBounds();
    text.setOrigin(textBounds.left + textBounds.width / 2.0f, textBounds.top + textBounds.height / 2.0f);
    text.setPosition(pixPos);
    window.draw(text);
}

void Player::setPosition(float x, float y) { pos.x = x; pos.y = y; }
sf::Vector2f Player::getPosition() const { return pos; }
sf::Vector2f Player::getVelocity() const { return velocity; }
void Player::setVelocity(sf::Vector2f v) { velocity = v; }
float Player::getRadius() const { return radius; }
Action Player::getLastAction() const { return lastAction; }
