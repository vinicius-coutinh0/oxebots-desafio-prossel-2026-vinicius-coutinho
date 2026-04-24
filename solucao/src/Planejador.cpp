#include "Planejador.h"

namespace Planejador {

// Peso com que o termo repulsivo é somado ao atrativo.
constexpr float PESO_REPULSAO = 1.2f;

Vec2 calcularDirecao(const Vec2& origem,
                     const Vec2& alvo,
                     const std::vector<Vec2>& obstaculos,
                     float raioRepulsao) {
    // Atração: versor apontando para o alvo.
    Vec2 vetorAtrativo = (alvo - origem).normalizado();

    // Repulsão: soma dos vetores que apontam dos obstáculos para o robo,
    // ponderada por proximidade. Só age dentro de `raioRepulsao`.
    Vec2 vetorRepulsivo{0.0f, 0.0f};
    for (const auto& obstaculo : obstaculos) {
        Vec2 afastamento = origem - obstaculo;
        float distanciaAtual = afastamento.norma();
        if (distanciaAtual > 1e-4f && distanciaAtual < raioRepulsao) {
            // Peso linear: 1 quando colado, 0 quando sai do raio.
            float pesoProximidade = (raioRepulsao - distanciaAtual) / raioRepulsao;
            vetorRepulsivo += afastamento.normalizado() * pesoProximidade;
        }
    }

    Vec2 direcaoResultante = vetorAtrativo + vetorRepulsivo * PESO_REPULSAO;
    return direcaoResultante.normalizado();
}

Action direcaoParaAction(const Vec2& direcao) {
    Action acao;
    acao.moveDirectionX = direcao.x;
    acao.moveDirectionY = direcao.y;
    return acao;
}

std::vector<Vec2> coletarObstaculos(const GameState& state) {
    std::vector<Vec2> obstaculos;
    obstaculos.reserve(state.teammates.size() + state.opponents.size());

    // Aliados (exceto o próprio robô).
    for (size_t i = 0; i < state.teammates.size(); ++i) {
        if (static_cast<int>(i) == state.myIndex) continue;
        obstaculos.push_back({state.teammates[i].x, state.teammates[i].y});
    }

    // Todos os adversários.
    for (const auto& adversario : state.opponents) {
        obstaculos.push_back({adversario.x, adversario.y});
    }
    return obstaculos;
}

}
