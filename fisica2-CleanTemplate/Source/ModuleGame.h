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
    // Texturas
    Texture2D texMap{};             // tablero
    Texture2D texBall{};            // bola (referencia)
    Texture2D texFlipLeft{};        // palanca_inverted.png  (IZQUIERDA)
    Texture2D texFlipRight{};       // palanca.png           (DERECHA)

    // Centros (como en el proyecto de tus compis)
    Vector2 flipLeftCenter{ 0,0 };    // (210 - w/2, 604 + h/2)
    Vector2 flipRightCenter{ 0,0 };   // (298 - w/2, 604 + h/2)

    // Ángulos (arranca 0 para evitar “X”; después ajustamos si quieres)
    float flipLeftAngle = 0.0f;
    float flipRightAngle = 0.0f;

    // Bola solo de referencia visual
    Vector2 posBall{ 300.0f, 200.0f };

    void AdjustWindowToMap();
    void DrawCentered(Texture2D& tex, Vector2 center, float rotationDeg);
    inline bool IsLoaded(const Texture2D& t) const { return t.id != 0; }
};
