#include "Estrategia.h"
#include "Planejador.h"
#include "Vetor.h"
#include <cmath>

namespace {

// =========================================================================
//                           Constantes de campo
// =========================================================================

// Raio do robô arredondado em metros
constexpr float RAIO_ROBO = 0.04f;

// Meio-comprimento do campo em X. Os gols ficam em x = ±X_GOL.
constexpr float X_GOL = 0.85f;

// =========================================================================
//                           Constantes da FSM
// =========================================================================

// Perseguir → Empurrar: bola encostada e quase alinhada com o gol.
constexpr float DISTANCIA_ENTRADA_EMPURRAR = 0.10f;
constexpr float ANGULO_ENTRADA_EMPURRAR    = 0.52f; 

// Saída de Empurrar com margens maiores. 
// Evita flip-flop quando a bola fica na fronteira:
//   - distância > 16 cm: perdeu contato;
//   - ângulo > 40 graus: desalinhado: arrasta em vez de empurrar.
constexpr float DISTANCIA_SAIDA_EMPURRAR = 0.16f;
constexpr float ANGULO_SAIDA_EMPURRAR    = 0.70f;

// Atacante recua quando a bola está no fundo da defesa e o ala
// está tão perto da bola quanto ele (margem de 15 cm). Evita
// atacante e ala juntos no mesmo lance defensivo.
constexpr float X_FUNDO_DEFESA        = -0.30f;
constexpr float MARGEM_CEDER_DISPUTA  =  0.15f;

// Saída de Recuar: bola saiu da nossa profunda.
constexpr float X_SAIDA_RECUAR = -0.15f;

// =========================================================================
//                          Helper
// =========================================================================

// Ângulo entre dois vetores em radianos (via produto escalar).
// Retorna 0 se algum vetor é nulo.
float anguloEntre(const Vec2& vetorA, const Vec2& vetorB) {
    float normaA = vetorA.norma();
    float normaB = vetorB.norma();
    if (normaA < 1e-6f || normaB < 1e-6f) return 0.0f;
    float cosseno = (vetorA.x * vetorB.x + vetorA.y * vetorB.y)
                  / (normaA * normaB);
    cosseno = clamp(cosseno, -1.0f, 1.0f);
    return std::acos(cosseno);
}

// =========================================================================
//                   Contexto
// =========================================================================

// Estrutura auxiliar com informações pré-computadas para a FSM do atacante.
// Centraliza derivações usadas múltiplas vezes por frame.
struct Contexto {
    Vec2  eu;               // posição do robô
    Vec2  bola;
    Vec2  golAdversario;    // em x = +X_GOL se ataco para +x, senão -X_GOL
    Vec2  golProprio;
    float sinalAtaque;      // +1 se ataco para +x, -1 caso contrário
    float distanciaEuBola;
    float distanciaAlaBola; // usado apenas pela FSM do atacante
};

Contexto montarContexto(const GameState& state, bool ehTimeA) {
    Contexto contexto;
    const EntityState& eu = state.getMe();
    contexto.eu = {eu.x, eu.y};
    contexto.bola = {state.ball.x, state.ball.y};
    contexto.sinalAtaque = ehTimeA ? 1.0f : -1.0f;
    contexto.golAdversario = { contexto.sinalAtaque * X_GOL, 0.0f};
    contexto.golProprio    = {-contexto.sinalAtaque * X_GOL, 0.0f};
    contexto.distanciaEuBola = distancia(contexto.eu, contexto.bola);

    const Vec2 posicaoAla{state.teammates[1].x, state.teammates[1].y};
    contexto.distanciaAlaBola = distancia(posicaoAla, contexto.bola);
    return contexto;
}

// =========================================================================
//                           FSM do atacante
// =========================================================================

EstadoAtacante transicaoEstado(EstadoAtacante atual, const Contexto& contexto) {
    const float xBolaRelativo = contexto.bola.x * contexto.sinalAtaque;
    const float xEuRelativo   = contexto.eu.x   * contexto.sinalAtaque;

    // Override: bola no nosso fundo + ala já disputando → recuar.
    const bool bolaFundoDefesa = xBolaRelativo < X_FUNDO_DEFESA;
    const bool alaNaDisputa    = contexto.distanciaAlaBola
                               < contexto.distanciaEuBola + MARGEM_CEDER_DISPUTA;
    if (bolaFundoDefesa && alaNaDisputa) {
        return EstadoAtacante::Recuar;
    }

    switch (atual) {
        case EstadoAtacante::Perseguir: {
            Vec2 paraBola = contexto.bola - contexto.eu;
            Vec2 paraGol  = contexto.golAdversario - contexto.eu;
            float angulo = anguloEntre(paraBola, paraGol);
            if (contexto.distanciaEuBola < DISTANCIA_ENTRADA_EMPURRAR
                && angulo < ANGULO_ENTRADA_EMPURRAR) {
                return EstadoAtacante::Empurrar;
            }
            return EstadoAtacante::Perseguir;
        }
        case EstadoAtacante::Empurrar: {
            // "Bola passou por mim": ela está para trás do atacante na
            // direção do gol.
            bool bolaPassouPorMim = xBolaRelativo < xEuRelativo - 0.03f;
            Vec2 paraBola = contexto.bola - contexto.eu;
            Vec2 paraGol  = contexto.golAdversario - contexto.eu;
            float angulo = anguloEntre(paraBola, paraGol);
            if (contexto.distanciaEuBola > DISTANCIA_SAIDA_EMPURRAR
                || bolaPassouPorMim
                || angulo > ANGULO_SAIDA_EMPURRAR) {
                return EstadoAtacante::Perseguir;
            }
            return EstadoAtacante::Empurrar;
        }
        case EstadoAtacante::Recuar: {
            if (xBolaRelativo > X_SAIDA_RECUAR) {
                return EstadoAtacante::Perseguir;
            }
            return EstadoAtacante::Recuar;
        }
    }
    return atual;
}

// =========================================================================
//                  Cálculo de alvo por estado do atacante
// =========================================================================

// Ponto de aproximação para o estado Perseguir.
//
// Alvo base: atrás da bola, alinhado com o gol adversário. Ao se mover
// em direção ao alvo, o atacante naturalmente chega posicionado para
// empurrar a bola ao gol no frame seguinte.
//
// Dois ajustes especiais:
//
// 1. Canto: se o ponto atrás da bola ficaria FORA do campo (bola encostada
//    em parede), usa aproximação perpendicular pelo lado do centro do
//    campo. Sem isso, o atacante bate na parede tentando chegar em um
//    ponto inalcançável.
//
// 2. Adversário grudado na bola (< 8 cm): aproxima-se por um ponto de
//    flanco (perpendicular ao eixo bola→gol), do lado OPOSTO ao adversário.
//    Em duas fases: primeiro o flanco, depois comita direto na bola.
//    Sem a segunda fase, a zona morta pararia o atacante no flanco.
Vec2 pontoDeAtaque(const Contexto& contexto, const GameState& state) {
    Vec2 direcaoGol = (contexto.golAdversario - contexto.bola).normalizado();
    Vec2 pontoAtrasBola = contexto.bola - direcaoGol * (RAIO_ROBO + 0.04f);

    // --- Ajuste 1: escape de canto ---
    const float limiteX = 0.80f;
    const float limiteY = 0.60f;
    bool foraDoCampo = std::abs(pontoAtrasBola.x) > limiteX
                    || std::abs(pontoAtrasBola.y) > limiteY;
    if (foraDoCampo) {
        Vec2 perpendicular{-direcaoGol.y, direcaoGol.x};
        Vec2 paraCentro = Vec2{0.0f, 0.0f} - contexto.bola;
        float projecao = perpendicular.x * paraCentro.x
                       + perpendicular.y * paraCentro.y;
        float sinal = (projecao > 0.0f) ? 1.0f : -1.0f;
        return contexto.bola + perpendicular * sinal * 0.11f;
    }

    // Se já está próximo da bola, não vale a pena flanquear.
    if (contexto.distanciaEuBola < 0.12f) return pontoAtrasBola;

    // --- Ajuste 2: adversário grudado na bola ---
    for (const auto& adversario : state.opponents) {
        Vec2 posicaoAdversario{adversario.x, adversario.y};
        if (distancia(posicaoAdversario, contexto.bola) < 0.08f) {
            Vec2 perpendicular{-direcaoGol.y, direcaoGol.x};
            Vec2 bolaParaAdversario = posicaoAdversario - contexto.bola;
            float projecao = bolaParaAdversario.x * perpendicular.x
                           + bolaParaAdversario.y * perpendicular.y;
            float sinal = (projecao > 0.0f) ? -1.0f : 1.0f;
            Vec2 flanco = contexto.bola
                        + perpendicular * sinal * 0.10f
                        - direcaoGol * 0.02f;

            // Fase 2: chegou no flanco → ataca a bola.
            if (distancia(contexto.eu, flanco) < 0.04f) {
                return contexto.bola;
            }
            return flanco;
        }
    }

    return pontoAtrasBola;
}

// Alvo do estado Empurrar. Mira no canto do gol oposto ao goleiro adversário.
// Exceção: bola encurralada no canto ofensivo com adversário bloqueando
// -> retira a bola para o meio-campo e reorganiza o ataque.
Vec2 alvoEmpurrar(const Contexto& contexto, const GameState& state) {
    float yCanto = 0.0f;
    if (!state.opponents.empty()) {
        float yGoleiro = state.opponents[0].y;
        yCanto = (yGoleiro >= 0.0f) ? -0.15f : 0.15f;
    }

    // Escape de canto morto: bola colada na linha de fundo ofensiva,
    float xBolaRelativo = contexto.bola.x * contexto.sinalAtaque;
    bool bolaEncurralada = xBolaRelativo > 0.70f
                        && std::abs(contexto.bola.y) > 0.40f;
    if (bolaEncurralada) {
        float menorDistancia = 1e9f;
        for (const auto& adversario : state.opponents) {
            float distanciaAdversarioBola =
                distancia({adversario.x, adversario.y}, contexto.bola);
            if (distanciaAdversarioBola < menorDistancia) {
                menorDistancia = distanciaAdversarioBola;
            }
        }
        if (menorDistancia < 0.25f) {
            return { contexto.bola.x - contexto.sinalAtaque * 0.25f, 0.0f };
        }
    }

    return { contexto.golAdversario.x, yCanto };
}

Vec2 alvoPorEstado(EstadoAtacante estado,
                   const Contexto& contexto,
                   const GameState& state) {
    switch (estado) {
        case EstadoAtacante::Perseguir:
            return pontoDeAtaque(contexto, state);
        case EstadoAtacante::Empurrar:
            return alvoEmpurrar(contexto, state);
        case EstadoAtacante::Recuar:
            // Posição de cobertura no meio-campo, acompanhando levemente
            // o eixo Y da bola.
            return { -contexto.sinalAtaque * 0.20f, contexto.bola.y * 0.5f };
    }
    return contexto.bola;
}

// =========================================================================
//                         Decisões por papel
// =========================================================================

// Goleiro. Fica no próprio gol, acompanhando a bola em Y com pequena
// predição para antecipar chutes. Sai apenas se a bola estiver muito
// próxima do gol e a ameaça for centralizada.
Vec2 decisaoGoleiro(const GameState& state,
                    float sinalAtaque,
                    float sinalDefesa) {
    const EntityState& eu = state.getMe();
    const EntityState& bola = state.ball;
    const float xPosicaoGoleiro = sinalDefesa * 0.75f;

    const float tempoPredicao = 0.10f;
    float yPredito = bola.y + bola.vy * tempoPredicao;
    float yAlvo = clamp(yPredito, -0.12f, 0.12f);

    // Condição de saída para cortar a bola na área.
    float xBolaRelativo = bola.x * sinalAtaque;
    float distanciaBolaGol = distancia({bola.x, bola.y},
                                        {xPosicaoGoleiro, 0.0f});
    bool bolaNaArea = xBolaRelativo < -0.65f && std::abs(bola.y) < 0.25f;

    const Vec2 posicaoAla{state.teammates[1].x, state.teammates[1].y};
    float distanciaEuBola  = distancia({eu.x, eu.y}, {bola.x, bola.y});
    float distanciaAlaBola = distancia(posicaoAla,   {bola.x, bola.y});
    bool souMaisProximo = distanciaEuBola < distanciaAlaBola;

    if (bolaNaArea && souMaisProximo && distanciaBolaGol < 0.20f) {
        return {bola.x, bola.y};
    }
    return {xPosicaoGoleiro, yAlvo};
}

// Ala. Três modos em ordem de prioridade:
//
// 1. Bola livre no meio: se estou mais perto que o atacante e nenhum
//    adversário a disputa, eu avanço para pegá-la.
//
// 2. Apoio ofensivo: quando a bola está no campo adversário,
//    posiciono-me 15 cm atrás dela, no lado OPOSTO ao atacante.
//    Cria dois ângulos de ataque e cobre rebote.
//
// 3. Defesa: entre a bola e o nosso gol. Se adversário tem posse,
//    pressão no adversário: (12 cm à frente dele); senão, linha de corte
//    (20 cm da bola). X limitado à faixa intermediária para não invadir
//    o gol nem subir demais.
Vec2 decisaoAla(const GameState& state,
                float sinalAtaque,
                float sinalDefesa) {
    const EntityState& eu = state.getMe();
    const Vec2 posicaoEu{eu.x, eu.y};
    const Vec2 bola{state.ball.x, state.ball.y};
    const float xBolaRelativo = bola.x * sinalAtaque;

    const Vec2 posicaoAtacante{state.teammates[2].x, state.teammates[2].y};
    float distanciaEuBola       = distancia(posicaoEu,       bola);
    float distanciaAtacanteBola = distancia(posicaoAtacante, bola);
    bool souMaisProximoQueAtacante =
        distanciaEuBola < distanciaAtacanteBola - 0.05f;

    // Adversário mais próximo da bola.
    Vec2 adversarioPerigoso = bola;
    float distanciaAdversarioBola = 1e9f;
    for (const auto& adversario : state.opponents) {
        float d = distancia({adversario.x, adversario.y}, bola);
        if (d < distanciaAdversarioBola) {
            distanciaAdversarioBola = d;
            adversarioPerigoso = {adversario.x, adversario.y};
        }
    }

    // --- Modo 1: bola livre  ---
    if (souMaisProximoQueAtacante
        && distanciaAdversarioBola > 0.10f
        && xBolaRelativo < 0.20f) {
        Vec2 direcaoGol = (Vec2{sinalAtaque * X_GOL, 0.0f} - bola).normalizado();
        return bola - direcaoGol * 0.06f;
    }

    // --- Modo 2: apoio ofensivo ---
    if (xBolaRelativo > 0.10f) {
        float ladoAtacante = (posicaoAtacante.y >= bola.y) ? 1.0f : -1.0f;
        float yApoio = bola.y - ladoAtacante * 0.22f;
        float xApoio = sinalAtaque * (xBolaRelativo - 0.15f);
        return {clamp(xApoio, -0.55f, 0.55f),
                clamp(yApoio, -0.50f, 0.50f)};
    }

    // --- Modo 3: defesa ---
    const Vec2 nossoGol{sinalDefesa * X_GOL, 0.0f};
    Vec2 pontoDefesa;
    if (distanciaAdversarioBola < 0.10f && xBolaRelativo < -0.05f) {
        // Pressão direta: 12 cm à frente do adversário, linha do nosso gol.
        Vec2 direcaoAdversario = (nossoGol - adversarioPerigoso).normalizado();
        pontoDefesa = adversarioPerigoso + direcaoAdversario * 0.12f;
    } else {
        // Linha de corte: 20 cm da bola em direção ao nosso gol.
        Vec2 direcaoBolaGol = (nossoGol - bola).normalizado();
        pontoDefesa = bola + direcaoBolaGol * 0.20f;
    }

    // Faixa permitida de X: da linha intermediária até a defesa, mas
    // sem invadir a área do goleiro nem subir demais em Y.
    float xMin = sinalDefesa * 0.65f;
    float xMax = sinalDefesa * 0.10f;
    if (sinalDefesa < 0) {
        pontoDefesa.x = clamp(pontoDefesa.x, xMin, xMax);
    } else {
        pontoDefesa.x = clamp(pontoDefesa.x, xMax, xMin);
    }
    pontoDefesa.y = clamp(pontoDefesa.y, -0.55f, 0.55f);
    return pontoDefesa;
}

// Atacante. Usa a FSM: primeiro transiciona o estado, depois calcula
// o alvo correspondente. O estado é passado por referência porque
// persiste entre frames.
Vec2 decisaoAtacante(EstadoAtacante& estado,
                     const GameState& state,
                     bool ehTimeA) {
    Contexto contexto = montarContexto(state, ehTimeA);
    estado = transicaoEstado(estado, contexto);
    return alvoPorEstado(estado, contexto, state);
}

} 

// =========================================================================
//                          Classe Estrategia
// =========================================================================

Estrategia::Estrategia(int id, bool ehTimeA) : id(id), ehTimeA(ehTimeA) {
    if      (id == 0) papel = Papel::Goleiro;
    else if (id == 1) papel = Papel::Ala;
    else              papel = Papel::Atacante;
}

Action Estrategia::think(const GameState& state) {
    const EntityState& eu = state.getMe();
    const Vec2 origem{eu.x, eu.y};

    const float sinalAtaque = ehTimeA ?  1.0f : -1.0f;
    const float sinalDefesa = -sinalAtaque;

    Vec2 alvo = origem;
    switch (papel) {
        case Papel::Goleiro:
            alvo = decisaoGoleiro(state, sinalAtaque, sinalDefesa);
            break;
        case Papel::Ala:
            alvo = decisaoAla(state, sinalAtaque, sinalDefesa);
            break;
        case Papel::Atacante:
            alvo = decisaoAtacante(estadoAtacante, state, ehTimeA);
            break;
    }

    auto obstaculos = Planejador::coletarObstaculos(state);
    Vec2 direcao = Planejador::calcularDirecao(origem, alvo, obstaculos);

    // Zona morta: evita tremor quando chega no alvo. 
    // Desligada para o atacante, cujo alvo é sempre dinâmico (bola)
    if (papel != Papel::Atacante && (alvo - origem).norma() < 0.02f) {
        return {0.0f, 0.0f};
    }
    return Planejador::direcaoParaAction(direcao);
}
