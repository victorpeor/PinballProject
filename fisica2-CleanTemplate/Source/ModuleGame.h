#pragma once

#include "Globals.h"
#include "Module.h"
#include "p2Point.h"
#include "raylib.h"

class ModuleGame : public Module
{
public:
    ModuleGame(Application* app, bool start_enabled = true);
    ~ModuleGame();

    bool Start() override;
    update_status Update() override;
    bool CleanUp() override;

private:
    // === Texturas ===
    Texture2D texMap{};        // Fondo del tablero
    Texture2D texBall{};       // Bola (referencia visual)
    Texture2D texFlipLeft{};   // Palanca izquierda  (palanca_inverted.png)
    Texture2D texFlipRight{};  // Palanca derecha   (palanca.png)

    // === Centros de cada palanca (ajustados ligeramente hacia abajo) ===
    Vector2 leftCenter{ 0.0f, 0.0f };
    Vector2 rightCenter{ 0.0f, 0.0f };

    // === Rotaciones (grados) — reposo mirando hacia abajo/centro ===
    float leftAngleDeg = 24.0f;  // Izquierda
    float rightAngleDeg = -24.0f;  // Derecha

    // Bola (solo referencia)
    Vector2 posBall{ 300.0f, 200.0f };

    // === Helpers ===
    void AdjustWindowToMap();
    void DrawCentered(Texture2D& tex, Vector2 center, float rotationDeg);

    inline bool IsLoaded(const Texture2D& tex) const { return tex.id != 0; }
};
