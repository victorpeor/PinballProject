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
    Texture2D texMap{};
    Texture2D texBall{};
    Texture2D texFlipLeft{};   // palanca_inverted.png (izquierda)
    Texture2D texFlipRight{};  // palanca.png (derecha)

    // Centros “buenos” (como en tu snippet)
    Vector2 leftCenter{ 0,0 };
    Vector2 rightCenter{ 0,0 };

    // Pivote mundo (punta interna fija)
    Vector2 leftPivotWorld{ 0,0 };
    Vector2 rightPivotWorld{ 0,0 };

    // Pivote local (px dentro del sprite) = punta interna
    Vector2 leftLocalPivotPx{ 0,0 };
    Vector2 rightLocalPivotPx{ 0,0 };

    // Ángulos y movimiento
    float leftAngleDeg = 24.0f;
    float rightAngleDeg = -24.0f;
    const float leftRestDeg = 24.0f;
    const float leftHitDeg = -50.0f;
    const float rightRestDeg = -24.0f;
    const float rightHitDeg = 50.0f;
    const float flipSpeedDps = 600.0f;

    Vector2 posBall{ 300.0f, 200.0f };

    void AdjustWindowToMap();
    void DrawWithPivot(const Texture2D& tex, Vector2 worldPivot, Vector2 localPivotPx, float rotationDeg) const;
    inline bool IsLoaded(const Texture2D& t) const { return t.id != 0; }
};
