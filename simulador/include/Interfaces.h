#ifndef INTERFACES_H
#define INTERFACES_H

#include <vector>
#include <cmath>

// Dimensões Oficiais (em metros)
const float FIELD_WIDTH = 1.70f;
const float FIELD_HEIGHT = 1.30f;
const float GOAL_WIDTH = 0.10f;
const float GOAL_HEIGHT = 0.40f;

struct EntityState {
    float x, y; // Posição em metros (0,0 é o centro)
    float vx, vy; // Velocidade em m/s

    // Helper: Distância até outra entidade ou ponto
    float distTo(float targetX, float targetY) const {
        return std::sqrt(std::pow(targetX - x, 2) + std::pow(targetY - y, 2));
    }

    // Helper: Ângulo até outra entidade ou ponto (em radianos)
    float angleTo(float targetX, float targetY) const {
        return std::atan2(targetY - y, targetX - x);
    }
};

struct GameState {
    EntityState ball;
    std::vector<EntityState> teammates;
    std::vector<EntityState> opponents;
    int myIndex; // Índice do robô atual (0, 1 ou 2)

    // Helper: Retorna os dados do próprio robô que está executando a lógica
    const EntityState& getMe() const {
        return teammates[myIndex];
    }
};

struct Action {
    float moveDirectionX; // -1.0 a 1.0
    float moveDirectionY; // -1.0 a 1.0
};

class PlayerAgent {
public:
    virtual ~PlayerAgent() = default;
    virtual Action think(const GameState& state) = 0;
};

#endif // INTERFACES_H
