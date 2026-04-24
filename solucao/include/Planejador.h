#ifndef PLANEJADOR_H
#define PLANEJADOR_H

#include "Vetor.h"
#include "Interfaces.h"
#include <vector>

// Planejador de caminho baseado em campo de vetores:
//   - vetor atrativo: na direção do alvo;
//   - vetor repulsivo: próximo a outros robôs.
//
// O resultado é a soma normalizada dos dois vetores, que vira a direção
// de movimento do robô no frame atual.
namespace Planejador {

    // Calcula a direção de movimento (versor) que combina atração ao
    // alvo e repulsão de obstáculos próximos.
    //
    // Parâmetros:
    //   origem:          posição atual do robô.
    //   alvo:            posição desejada.
    //   obstaculos:      posições de aliados e adversários a evitar.
    //   raioRepulsao:    a repulsão só age abaixo desta distância.
    Vec2 calcularDirecao(const Vec2& origem,
                         const Vec2& alvo,
                         const std::vector<Vec2>& obstaculos,
                         float raioRepulsao = 0.08f);

    // Converte um versor de direção na Action esperada pelo simulador.
    Action direcaoParaAction(const Vec2& direcao);

    // Coleta as posições dos outros robôs em campo (aliados sem contar
    // a si mesmo + todos os adversários) para alimentar calcularDirecao.
    std::vector<Vec2> coletarObstaculos(const GameState& state);

}

#endif
