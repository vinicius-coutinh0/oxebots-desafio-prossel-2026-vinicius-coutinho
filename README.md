# Desafio de Prossel: Simulador de Futebol 2D (Padrão VSSS)

Bem-vindo ao desafio de programação de robôs! Seu objetivo é implementar a lógica de comportamento de uma equipe de 3 robôs em um simulador que segue as proporções reais da categoria VSSS (Very Small Size Soccer).

---

## 🚀 Como Iniciar no Windows (Recomendado)

O projeto está configurado para funcionar de forma automatizada no Windows utilizando o **Visual Studio 2022**. Toda a sua lógica deve ser implementada dentro da pasta `solucao/`.

### 📦 Pré-requisitos
1. Instale o **[Visual Studio 2022 Community](https://visualstudio.microsoft.com/pt-br/vs/community/)**.
2. Na instalação, selecione a carga de trabalho: **"Desenvolvimento para desktop com C++"**.
3. O Visual Studio já inclui o CMake e os compiladores necessários.

### 🛠️ Compilação e Execução Passo a Passo
1. Abra o Visual Studio 2022.
2. Selecione **"Abrir uma Pasta Local"** e escolha a raiz deste projeto.
3. O Visual Studio detectará o arquivo `CMakeLists.txt` automaticamente e iniciará a "Geração do Cache do CMake" (acompanhe na barra de status inferior).
4. **Importante:** Aguarde a mensagem "Geração do cache do CMake concluída".
5. No menu superior, clique em **"Compilar" > "Compilar Tudo"** (ou use `Ctrl + Shift + B`).
   - *Nota:* O projeto está configurado para copiar automaticamente todas as DLLs e fontes necessárias para a pasta de saída.
6. Selecione `SimuladorFutebol.exe` no menu de inicialização (ao lado do botão verde de play) e aperte **F5**.
7. O simulador abrirá uma janela gráfica. Todos os robôs já estarão executando o seu código do método `think()`.

---

## 🧠 Como Programar

O ponto de entrada principal é a classe `Estrategia` nos arquivos:
- `solucao/include/Estrategia.h`
- `solucao/src/Estrategia.cpp`

O simulador chama `think(const GameState& state)` para cada robô. Você tem acesso aos seguintes dados:
- **Seu Robô:** `state.getMe()` (Posição `x, y` e Velocidade `vx, vy`).
- **A Bola:** `state.ball` (Posição `x, y` e Velocidade `vx, vy`).
- **Aliados e Inimigos:** Listas `state.teammates` e `state.opponents`.

```cpp
Action Estrategia::think(const GameState& state) {
    Action a;
    const EntityState& eu = state.getMe();
    const EntityState& bola = state.ball;

    // Exemplo: Como mover para um ponto (Ex: x=0.4, y=0.2)
    float alvoX = 0.4f;
    float alvoY = 0.2f;

    float angulo = eu.angleTo(alvoX, alvoY);
    a.moveDirectionX = std::cos(angulo);
    a.moveDirectionY = std::sin(angulo);

    return a;
}
```

---

## 🎯 Requisitos da Solução

Espera-se que você implemente duas camadas lógicas principais:

1.  **Tomador de Decisão:** Uma estrutura que determine o que cada robô deve fazer em cada momento (ex: "Ir para a bola", "Ficar na defesa", "Bloquear oponente"). Você pode usar **Máquinas de Estados (FSM)** ou lógicas similares.
2.  **Planejador de Caminho (Planner):** Uma lógica que calcule o movimento necessário para atingir o objetivo definido pelo Tomador de Decisão. 
    *   *Dica:* Considere o posicionamento correto para empurrar a bola e desvio básico de obstáculos.

---

## 📐 Unidades e Medidas (Sistema Internacional)

O simulador trabalha exclusivamente com **METROS** e **SEGUNDOS**.
- **Sistema de Coordenadas:** O centro do campo é o ponto `(0, 0)`.
- **Limites de X:** de `-0.85` até `+0.85`.
- **Limites de Y:** de `-0.65` até `+0.65`.
- **Gols:** Verticalmente entre `y = -0.20` e `y = +0.20`.

---

## 🛠️ Regras e Comandos
1.  **Duração:** As partidas duram 5 minutos (300 segundos).
2.  **Velocidade:** Robôs atingem até `0.5 m/s`.
3.  **Comandos de Teclado:**
    -   **Tecla `F`:** Marca falta (reseta as posições dos robôs).
    -   **Tecla `R`:** Reinicia o jogo completo (placar, tempo e posições).

---

## 📤 Entrega e Avaliação

Ao finalizar o desafio, você deve entregar um **repositório (GitHub/GitLab)** contendo:
1.  O código fonte da sua solução (pasta `solucao/`).
2.  Um documento explicativo (pode ser um `README_SOLUCAO.md` ou um `PDF`).

### 📝 O que deve conter no documento/apresentação:
- **Explicação da Lógica:** Detalhamento do seu **Tomador de Decisão** e Planejador.
- **Justificativa das Escolhas:** Por que você escolheu essa arquitetura? (Ex: Por que usar uma Máquina de Estados? Ou, caso tenha feito via lógica linear no código, por que essa escolha foi suficiente?).
- **Coordenação:** Como você lidou com a matemática e a cooperação entre os 3 robôs.
- **Apresentação:**: No dia combinado, você terá até 20 minutos para aprresentar sua solução e mais 5 para questionamentos da equipe. 

### 📊 Critérios de Avaliação:
- **Robustez:** Como os robôs reagem a diferentes situações.
- **Organização:** Código limpo e modularizado.
- **Matemática:** Uso de vetores, trigonometria e física.
- **Planejamento de Rota:** Eficiência na movimentação.
- **Apresentação:** Você deverá explicar sua solução para a equipe.

> **⚠️ Regra sobre IA:** O uso de IA não é proibido para auxílio, mas a solução **não deve ser feita inteiramente por IA**. Você deve ser capaz de explicar cada parte da lógica. O uso excessivo sem compreensão resultará em eliminação.

Boa sorte! ⚽🤖🚀

