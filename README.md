# README da Solução — Desafio de Prossel (Simulador VSSS)

## 1. Visão geral

A solução está em `solucao/` e divide-se em duas camadas, como pedido no enunciado:

| Camada              | Arquivos                              | Responsabilidade                                                |
|---------------------|----------------------------------------|-----------------------------------------------------------------|
| Tomador de Decisão  | `Estrategia.h`, `Estrategia.cpp`      | Decide o alvo (ponto no campo) de cada robô em cada frame.     |
| Planejador          | `Planejador.h`, `Planejador.cpp`      | Converte `(origem, alvo, obstáculos)` em uma direção unitária. |
| Utilitários         | `Vetor.h`                              | `Vec2`, `distancia`, `clamp`.                                   |

Fluxo resumido por frame:

```
think(state) ─► decide papel (Goleiro/Ala/Atacante)
             ─► calcula alvo conforme o papel
             ─► Planejador.calcularDirecao(origem, alvo, obstáculos)
             ─► Action(dirX, dirY)
```

## 2. Papéis fixos

Derivados do `id` no construtor (`Simulator.cpp` cria 3 robôs com id 0, 1, 2):

- **0 — Goleiro:** fica no próprio gol acompanhando a bola em Y.
- **1 — Ala:** meio-campo polivalente (defensivo, apoio ofensivo, toma bola livre).
- **2 — Atacante:** persegue a bola, conduz ao gol adversário.

`enum class Papel { Goleiro, Ala, Atacante }`s.

## 3. Tomador de Decisão

### 3.1 Atacante — Máquina de Estados Finita (FSM)

Três estado

```
         ┌──────────────────────────────────────────┐
         │                                          ▼
    ┌─────────┐   d<10cm ∧ ang<30°   ┌───────────┐
    │Perseguir│ ───────────────────► │  Empurrar │
    └─────────┘ ◄─────────────────── └───────────┘
         ▲      d>16cm ∨ ang>40° ∨
         │      bola passou por mim
         │
         │ xBolaRel>-0.15          (override global)
         │                bola no fundo + ala já na disputa
    ┌─────────┐                           │
    │ Recuar  │ ◄─────────────────────────┘
    └─────────┘
```

#### Alvos por estado

- **Perseguir:** `ponto_atras_bola = bola − versor(gol − bola) · 0.08`. Ao alcançá-lo, o atacante já fica naturalmente alinhado para empurrar a bola adiante.
  - **Ajuste 1 (canto):** se o `ponto_atras_bola` cai fora do campo, usa aproximação perpendicular pelo lado do centro.
  - **Ajuste 2 (flanco):** se há adversário a < 8 cm da bola, passa por um ponto perpendicular ao eixo bola → gol no lado oposto ao adversário. Duas fases: rota até o flanco; já perto, comita na bola.

- **Empurrar:** mira no canto do gol OPOSTO ao goleiro adversário. Como `state.opponents[0]` é sempre o goleiro (id 0), essa informação é gratuita.
  - **Exceção (canto morto):** bola colada na linha de fundo ofensiva com defensor por perto -> empurra a bola para trás, ao meio-campo, reorganizando o ataque.

- **Recuar:** posição de cobertura em `x = −sinalAtaque · 0.20`, `y = bola.y / 2`.

### 3.2 Goleiro — heurística linear

1. **Tracking vertical:** `yAlvo = clamp(bola.y + bola.vy · 0.10, −0.12, 0.12)`. 
2. **Saida:** se bola próxima ao gol e mais perto de mim do que do ala, sai para cortar.

### 3.3 Ala — três modos por prioridade

1. **Bola livre no meio:** se eu estou mais perto que o atacante (margem 5 cm) e nenhum adversário a disputa (dist > 10 cm), avanço para pegá-la.
2. **Apoio ofensivo (pincer):** bola no campo adversário → me posiciono 15 cm atrás dela, no lado oposto ao atacante. Cobre rebote e cria dois ângulos de ataque.
3. **Defesa:** entre a bola e o gol do time.
   - Adversário com posse (< 10 cm) -> pressão direta 12 cm à frente dele.
   - Bola livre -> linha de corte 20 cm da bola, em direção ao nosso gol.

X limitado à faixa `[sinal · 0.10, sinal · 0.65]` para não invadir nem o gol nem o terço ofensivo.

## 4. Planejador de caminho

Implementação simples baseada em campo de vetores:

- **Vetor atrativo:** `versor(alvo − origem)`.
- **Vetor repulsivo:** soma dos versores `origem − obstáculo` para cada obstáculo a < 8 cm, ponderada linearmente pela proximidade.
- Resultado: `versor(atrativo + repulsivo · 1.2)`.

## 5. Justificativa das escolhas

### Por que FSM no atacante e lógica linear nos demais?
O atacante é o único com comportamento "com memória útil": precisa lembrar se está "empurrando" ou "chegando" para não oscilar entre os dois na fronteira. A hysterese resolve isso. Goleiro e ala reavaliam do zero a cada frame a partir do `GameState`.

## 6. Coordenação entre os 3 robôs

A coordenação é **implícita**, via consulta ao `GameState`:

- **Atacante cede ao ala:** transita para `Recuar` se a bola está no nosso fundo e o ala está tão perto quanto ele (margem 15 cm). Evita 2-contra-1 na própria área.
- **Ala cede ao atacante:** só ativa o modo "bola livre" se estiver ao menos 5 cm mais perto que o atacante.
- **Goleiro cede ao ala:** só sai para cortar se for mais próximo da bola que o ala.
- **Pincer ofensivo:** ala se posiciona sempre no lado Y oposto ao atacante (baseado em `posicaoAtacante.y`).
- **Mira baseada no goleiro adversário:** o atacante conhece a posição do goleiro inimigo (`opponents[0]`) e sempre mira no canto oposto.

Matematica:

- **Ponto de ataque:** `bola − versor(gol − bola) · r` (vetor unitário × raio).
- **Predição de bola:** `bola + bola.vel · t`.
- **Alinhamento:** `acos(â · b̂)` com clamp para evitar NaN.
- **Perpendicular:** rotação de 90° via `(x, y) → (−y, x)`.
- **Escolha de lado por projeção:** `sign(p · perpendicular)` decide de qual lado contornar.

## 7. Bug conhecido — travamento na quina ofensiva

### Sintoma
Com a bola no canto  (ex.: `(+0.85, +0.65)`) e o ala/zagueiro adversário ao lado segurando-a, o atacante fica parado no ponto perpendicular de escape sem nunca tocá-la. O jogo trava nessa configuração.

### Causa
No estado `Perseguir`, quando o ponto atrás da bola alinhado ao gol cai fora do campo (bola encostada na parede), o código usa uma aproximação perpendicular. Esse ponto de escape fica a ~11 cm da bola. Ao chegar, o atacante zera o vetor `(alvo − origem)` e, mesmo sem zona morta no atacante, a direção resultante é nula. Se o adversário não move a bola, o atacante não avança mais. Porque o alvo é imutável enquanto a bola não se mexer.

### Tentativas de correção (todas revertidas)

#### Tentativa 1 — alvo direto na bola quando foraDoCampo
```cpp
if (foraDoCampo) return contexto.bola;
```
**Resultado:** funcionou no canto específico, mas voltou um bug anterior (atacante avançando livremente pela perpendicular no lado do adversário sem ser marcado. a ala e o goleiro adversários não defendiam). Revertido.

#### Tentativa 2 — alvo = bola + direção ao centro × 0.02
```cpp
Vec2 paraCentro = (Vec2{0,0} - bola).normalizado();
return bola + paraCentro * 0.02f;
```
**Resultado:** equivalente a chegar por dentro e empurrar a bola contra a parede. Voltou a causar deadlock simétrico entre blue e red no centro do campo (ambos tentavam o mesmo movimento ao se encontrarem). Revertido.

#### Tentativa 3 — fase 2 igual ao flanco
```cpp
Vec2 pontoEscape = bola + perp * sinal * 0.11f;
if (distancia(eu, pontoEscape) < 0.04f) return bola;
return pontoEscape;
```
**Resultado:** na prática, re-introduziu o problema da tentativa 1 (atacante conseguindo chegar agressivamente na bola no canto). Revertido após observação em jogo.

### Estado atual
A versão atual usa só o ponto perpendicular (sem fase 2). O travamento no canto ainda acontece ocasionalmente quando o adversário segura a bola no canto extremo. A frequência é baixa o suficiente para permitir que o jogo transcorra e gols aconteçam na maior parte do tempo..

### Ideias ainda não testadas
- **Timer de estagnação:** se o atacante está no escape há mais de N frames e a bola mal se moveu, força uma saída para o meio-campo por T segundos.
- **Chamada de apoio explícita:** ala também vai ao canto quando o atacante está preso lá há tempo, criando 2×1 local.
- **Aproximação paralela à parede:** em vez de perpendicular ao eixo bola → gol, aproximar em linha com a parede mais próxima e empurrar a bola ao longo dela.

## 8. Estrutura de arquivos

```
solucao/
├── include/
│   ├── Estrategia.h    # enum Papel, enum EstadoAtacante, classe Estrategia
│   ├── Planejador.h    # interface do planejador
│   └── Vetor.h         # Vec2, distancia, clamp
└── src/
    ├── Estrategia.cpp  # FSM + heurísticas + constantes documentadas
    └── Planejador.cpp  # campo atrativo + repulsivo
```

## 9. Compilação e execução

Ver `README.md` principal do projeto. A pasta `solucao/` é compilada automaticamente pelo `CMakeLists.txt` via `GLOB_RECURSE`, sem necessidade de listar os arquivos.
