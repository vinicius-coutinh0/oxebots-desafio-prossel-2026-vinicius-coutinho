#ifndef ESTRATEGIA_H
#define ESTRATEGIA_H

#include "Interfaces.h"


enum class Papel { Goleiro, Ala, Atacante };

enum class EstadoAtacante { Perseguir, Empurrar, Recuar };

// Lógica de decisão de um robô.
//
// - Goleiro e Ala usam heurísticas lineares baseadas no estado do jogo.
// - Atacante usa uma máquina de estados (EstadoAtacante) .
//
// O planejador de caminho (ver Planejador.h) é compartilhado: cada papel
// apenas decide um alvo, e o planejador calcula a direção final
// considerando obstáculos (aliados e adversários).
class Estrategia : public PlayerAgent {
public:
    Estrategia(int id, bool ehTimeA);

    Action think(const GameState& state) override;

private:
    int  id;
    bool ehTimeA;
    Papel papel;
    EstadoAtacante estadoAtacante = EstadoAtacante::Perseguir;
};

#endif
