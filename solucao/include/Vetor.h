#ifndef VETOR_H
#define VETOR_H

#include <cmath>

// Vetor 2D em coordenadas do mundo (metros).
//
// Convenção: (0, 0) é o centro do campo.
// x cresce para a direita, y cresce para baixo.
//
struct Vec2 {
    float x = 0.0f;
    float y = 0.0f;

    Vec2() = default;
    Vec2(float x, float y) : x(x), y(y) {}

    Vec2 operator+(const Vec2& outro) const { return {x + outro.x, y + outro.y}; }
    Vec2 operator-(const Vec2& outro) const { return {x - outro.x, y - outro.y}; }
    Vec2 operator*(float escalar)     const { return {x * escalar, y * escalar}; }
    Vec2& operator+=(const Vec2& outro) { x += outro.x; y += outro.y; return *this; }

    // Módulo (comprimento) do vetor.
    float norma() const { return std::sqrt(x * x + y * y); }

    // Módulo ao quadrado — evita o sqrt quando só se compara grandezas.
    float normaQuadrada() const { return x * x + y * y; }

    // Versor (vetor unitário) na mesma direção. Retorna (0, 0) se o
    // vetor original é degenerado (evita divisão por zero).
    Vec2 normalizado() const {
        float n = norma();
        if (n < 1e-6f) return {0.0f, 0.0f};
        return {x / n, y / n};
    }
};

// Distância euclidiana entre dois pontos.
inline float distancia(const Vec2& a, const Vec2& b) {
    return (a - b).norma();
}

// Restringe valor ao intervalo [minimo, maximo].
inline float clamp(float valor, float minimo, float maximo) {
    return valor < minimo ? minimo : (valor > maximo ? maximo : valor);
}

#endif
