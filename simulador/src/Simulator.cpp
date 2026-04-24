#include "Simulator.h"
#include "Estrategia.h"
#include <cmath>
#include <iostream>
#include <cstdio>

// Conversão: 400 pixels por metro (1.7m = 680px, 1.3m = 520px)
const float M2P = 400.0f;

Simulator::Simulator(unsigned int w, unsigned int h) 
    : window(sf::VideoMode(w, h), "Simulador VSSS - Desafio Prossel"), 
      ball(0.0213f), // Raio da bola em metros (2.13cm)
      width((float)w), height((float)h) {
    
    window.setFramerateLimit(60);
    
    // Tenta carregar a fonte de caminhos comuns para garantir que funcione no VS e no terminal
    bool fontLoaded = font.loadFromFile("arial.ttf") || 
                     font.loadFromFile("assets/arial.ttf") ||
                     font.loadFromFile("../assets/arial.ttf");

    if (!fontLoaded) {
        std::cerr << "[ERRO] Nao foi possivel encontrar 'arial.ttf'!" << std::endl;
        std::cerr << "Certifique-se de que o arquivo existe na pasta do .exe ou em /assets" << std::endl;
    } else {
        std::cout << "[OK] Fonte carregada com sucesso!" << std::endl;
    }

    scoreText.setFont(font); scoreText.setCharacterSize(24); scoreText.setFillColor(sf::Color::White);
    scoreText.setPosition(width / 2.0f - 100.0f, 5.0f);

    timerText.setFont(font); timerText.setCharacterSize(20); timerText.setFillColor(sf::Color::White);
    timerText.setPosition(width / 2.0f - 120.0f, height - 35.0f);

    // Configura o visual do campo (Grama)
    field.setSize(sf::Vector2f(FIELD_WIDTH * M2P, FIELD_HEIGHT * M2P));
    field.setOrigin(field.getSize().x / 2.0f, field.getSize().y / 2.0f);
    field.setPosition(width / 2.0f, height / 2.0f);
    field.setFillColor(sf::Color(34, 139, 34)); 
    field.setOutlineThickness(3.0f);
    field.setOutlineColor(sf::Color::White);

    centerLine.setSize(sf::Vector2f(2.0f, FIELD_HEIGHT * M2P));
    centerLine.setOrigin(1.0f, FIELD_HEIGHT * M2P / 2.0f);
    centerLine.setPosition(width / 2.0f, height / 2.0f);
    centerLine.setFillColor(sf::Color::White);

    centerCircle.setRadius(0.2f * M2P); // Círculo central de 20cm
    centerCircle.setOrigin(centerCircle.getRadius(), centerCircle.getRadius());
    centerCircle.setPosition(width / 2.0f, height / 2.0f);
    centerCircle.setFillColor(sf::Color::Transparent);
    centerCircle.setOutlineThickness(3.0f);
    centerCircle.setOutlineColor(sf::Color::White);

    // Áreas e Gols (Visual)
    areaA.setSize(sf::Vector2f(0.15f * M2P, 0.70f * M2P));
    areaA.setOrigin(0, areaA.getSize().y / 2.0f);
    areaA.setPosition(width / 2.0f - FIELD_WIDTH * M2P / 2.0f, height / 2.0f);
    areaA.setFillColor(sf::Color::Transparent);
    areaA.setOutlineThickness(3.0f);
    areaA.setOutlineColor(sf::Color::White);

    areaB.setSize(sf::Vector2f(0.15f * M2P, 0.70f * M2P));
    areaB.setOrigin(areaB.getSize().x, areaB.getSize().y / 2.0f);
    areaB.setPosition(width / 2.0f + FIELD_WIDTH * M2P / 2.0f, height / 2.0f);
    areaB.setFillColor(sf::Color::Transparent);
    areaB.setOutlineThickness(3.0f);
    areaB.setOutlineColor(sf::Color::White);

    goalA.setSize(sf::Vector2f(GOAL_WIDTH * M2P, GOAL_HEIGHT * M2P));
    goalA.setOrigin(goalA.getSize().x, goalA.getSize().y / 2.0f);
    goalA.setPosition(width / 2.0f - FIELD_WIDTH * M2P / 2.0f, height / 2.0f);
    goalA.setFillColor(sf::Color(100, 100, 100));

    goalB.setSize(sf::Vector2f(GOAL_WIDTH * M2P, GOAL_HEIGHT * M2P));
    goalB.setOrigin(0, goalB.getSize().y / 2.0f);
    goalB.setPosition(width / 2.0f + FIELD_WIDTH * M2P / 2.0f, height / 2.0f);
    goalB.setFillColor(sf::Color(100, 100, 100));

    resetPositions();

    // Adiciona 3 jogadores por equipe (Raio 3.75cm = 0.0375m)
    auto addPlayers = [&](std::vector<std::unique_ptr<Player>>& team, sf::Color color, bool isTeamA) {
        for (int i = 0; i < 3; ++i) {
            team.push_back(std::make_unique<Player>(i, 0.0375f, color, std::make_unique<Estrategia>(i, isTeamA)));
        }
    };
    addPlayers(teamA, sf::Color::Blue, true);
    addPlayers(teamB, sf::Color::Red, false);
    resetPositions();
}

void Simulator::resetPositions(bool resetBall) {
    if (resetBall) {
        ball.setPosition(0, 0); 
        ball.setVelocity(sf::Vector2f(0, 0));
    }
    
    if (teamA.size() >= 3) {
        teamA[0]->setPosition(-0.7f, 0);
        teamA[1]->setPosition(-0.4f, 0.3f);
        teamA[2]->setPosition(-0.4f, -0.3f);
        for(auto& p : teamA) p->setVelocity(sf::Vector2f(0,0));
    }
    if (teamB.size() >= 3) {
        teamB[0]->setPosition(0.7f, 0);
        teamB[1]->setPosition(0.4f, 0.3f);
        teamB[2]->setPosition(0.4f, -0.3f);
        for(auto& p : teamB) p->setVelocity(sf::Vector2f(0,0));
    }
}

void Simulator::run() {
    sf::Clock clock;
    while (window.isOpen()) {
        float dt = clock.restart().asSeconds();
        processEvents(); update(dt); render();
    }
}

void Simulator::processEvents() {
    sf::Event event;
    while (window.pollEvent(event)) {
        if (event.type == sf::Event::Closed) window.close();
        if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::R) { 
            scoreA = 0; scoreB = 0; gameTime = 0.0f; 
            std::cout << "[RESET] Jogo reiniciado!" << std::endl;
            resetPositions(); 
        }
        if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::F) { 
            std::cout << "[FALTA] Posições dos jogadores resetadas!" << std::endl;
            resetPositions(false); 
        }
    }
}

GameState Simulator::getGameState(size_t playerIndex, bool isTeamA) {
    GameState gs;
    gs.ball = {ball.getPosition().x, ball.getPosition().y, ball.getVelocity().x, ball.getVelocity().y};
    auto& myTeam = isTeamA ? teamA : teamB; 
    auto& otherTeam = isTeamA ? teamB : teamA;
    for (auto& p : myTeam) gs.teammates.push_back({p->getPosition().x, p->getPosition().y, p->getVelocity().x, p->getVelocity().y});
    for (auto& p : otherTeam) gs.opponents.push_back({p->getPosition().x, p->getPosition().y, p->getVelocity().x, p->getVelocity().y});
    gs.myIndex = (int)playerIndex;
    return gs;
}

void Simulator::update(float dt) {
    if (gameTime >= 300.0f) { 
        if (timerText.getString() != "FIM! R p/ Reset") {
            std::cout << "[FIM] Tempo esgotado! Placar Final: Azul " << scoreA << " - " << scoreB << " Vermelho" << std::endl;
            timerText.setString("FIM! R p/ Reset"); 
        }
        return; 
    }
    gameTime += dt;
    scoreText.setString("Azul " + std::to_string(scoreA) + " - " + std::to_string(scoreB) + " Vermelho");
    int minutes = (int)gameTime / 60; int seconds = (int)gameTime % 60;
    char timeStr[64]; sprintf(timeStr, "%02d:%02d", minutes, seconds);
    timerText.setString(timeStr);

    for (size_t i = 0; i < teamA.size(); ++i) teamA[i]->update(getGameState(i, true), dt);
    for (size_t i = 0; i < teamB.size(); ++i) teamB[i]->update(getGameState(i, false), dt);
    ball.update(dt); handleCollisions();

    if (gameTime - lastLogTime > 0.1f) {
        lastLogTime = gameTime;
        printf("\r[METROS] Bola:(%.2f, %.2f) | Azul:", ball.getPosition().x, ball.getPosition().y);
        for(auto& p : teamA) printf(" (%.2f, %.2f)", p->getPosition().x, p->getPosition().y);
        fflush(stdout);
    }
}

void Simulator::handleCollisions() {
    sf::Vector2f bPos = ball.getPosition(); float bR = ball.getRadius(); sf::Vector2f bVel = ball.getVelocity();
    float limitX = FIELD_WIDTH / 2.0f;
    float limitY = FIELD_HEIGHT / 2.0f;

    // Colisão Y
    if (std::abs(bPos.y) + bR > limitY) {
        bVel.y *= -0.8f;
        ball.setPosition(bPos.x, (bPos.y > 0 ? limitY - bR : -limitY + bR));
    }

    // Colisão X (Gols e Paredes)
    if (std::abs(bPos.x) + bR > limitX) {
        // Verifica GOL
        if (std::abs(bPos.y) < GOAL_HEIGHT / 2.0f) {
            if (bPos.x > 0) scoreA++; else scoreB++;
            std::cout << "[GOL!] Placar: Azul " << scoreA << " - " << scoreB << " Vermelho" << std::endl;
            resetPositions();
            return;
        }
        bVel.x *= -0.8f;
        ball.setPosition((bPos.x > 0 ? limitX - bR : -limitX + bR), bPos.y);
    }
    ball.setVelocity(bVel);

    // Colisão Jogador vs Jogador
    auto checkPlayerPlayer = [&](std::unique_ptr<Player>& p1, std::unique_ptr<Player>& p2) {
        if (p1 == p2) return;
        sf::Vector2f pos1 = p1->getPosition(); sf::Vector2f pos2 = p2->getPosition();
        float dist = std::sqrt(std::pow(pos1.x - pos2.x, 2) + std::pow(pos1.y - pos2.y, 2));
        float minDist = p1->getRadius() + p2->getRadius();
        if (dist < minDist && dist > 0.0001f) {
            sf::Vector2f pushDir = (pos2 - pos1) / dist;
            float overlap = minDist - dist;
            p1->setPosition(pos1.x - pushDir.x * overlap * 0.5f, pos1.y - pushDir.y * overlap * 0.5f);
            p2->setPosition(pos2.x + pushDir.x * overlap * 0.5f, pos2.y + pushDir.y * overlap * 0.5f);
            sf::Vector2f v1 = p1->getVelocity();
            p1->setVelocity(p2->getVelocity() * 0.5f); p2->setVelocity(v1 * 0.5f);
        }
    };

    // Aplica colisões entre todos
    for(size_t i=0; i<teamA.size(); ++i) for(size_t j=i+1; j<teamA.size(); ++j) checkPlayerPlayer(teamA[i], teamA[j]);
    for(size_t i=0; i<teamB.size(); ++i) for(size_t j=i+1; j<teamB.size(); ++j) checkPlayerPlayer(teamB[i], teamB[j]);
    for(auto& pa : teamA) for(auto& pb : teamB) checkPlayerPlayer(pa, pb);

    // Colisão Jogador vs Parede
    auto checkPlayerWall = [&](std::unique_ptr<Player>& p) {
        sf::Vector2f pos = p->getPosition(); float r = p->getRadius(); sf::Vector2f vel = p->getVelocity();
        if (std::abs(pos.y) + r > limitY) { vel.y *= -0.5f; pos.y = (pos.y > 0 ? limitY - r : -limitY + r); }
        if (std::abs(pos.x) + r > limitX) { vel.x *= -0.5f; pos.x = (pos.x > 0 ? limitX - r : -limitX + r); }
        p->setPosition(pos.x, pos.y); p->setVelocity(vel);
    };
    for(auto& p : teamA) checkPlayerWall(p);
    for(auto& p : teamB) checkPlayerWall(p);

    // Colisão Jogador vs Bola
    auto checkPlayerBall = [&](std::unique_ptr<Player>& p) {
        sf::Vector2f pPos = p->getPosition(); sf::Vector2f bPos = ball.getPosition();
        float dist = std::sqrt(std::pow(pPos.x - bPos.x, 2) + std::pow(pPos.y - bPos.y, 2));
        float minDist = p->getRadius() + ball.getRadius();
        if (dist < minDist) {
            sf::Vector2f pushDir = (bPos - pPos) / dist;
            ball.setPosition(pPos.x + pushDir.x * minDist, pPos.y + pushDir.y * minDist);
            ball.setVelocity(p->getVelocity() * 1.5f);
        }
    };
    for (auto& p : teamA) checkPlayerBall(p);
    for (auto& p : teamB) checkPlayerBall(p);
}

void Simulator::render() {
    window.clear(sf::Color(50, 50, 50));
    
    // Todas as posições internas são em metros (0,0 no centro)
    // Para desenhar, convertemos: PixelX = WinWidth/2 + MeterX * M2P
    auto toPix = [&](sf::Vector2f mPos) -> sf::Vector2f {
        return sf::Vector2f(width / 2.0f + mPos.x * M2P, height / 2.0f + mPos.y * M2P);
    };

    window.draw(field); window.draw(centerLine); window.draw(centerCircle);
    window.draw(areaA); window.draw(areaB); window.draw(goalA); window.draw(goalB);
    window.draw(scoreText); window.draw(timerText);

    // Renderiza Bola e Jogadores
    sf::CircleShape bVis; bVis.setRadius(ball.getRadius() * M2P); bVis.setOrigin(bVis.getRadius(), bVis.getRadius());
    bVis.setPosition(toPix(ball.getPosition())); bVis.setFillColor(sf::Color::White);
    window.draw(bVis);

    for (auto& p : teamA) p->draw(window, font, false, toPix(p->getPosition()), M2P);
    for (auto& p : teamB) p->draw(window, font, false, toPix(p->getPosition()), M2P);
    
    window.display();
}
