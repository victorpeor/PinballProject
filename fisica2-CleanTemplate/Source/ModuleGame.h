#pragma once
#include "Globals.h"
#include "Module.h"
#include "p2Point.h"
#include "raylib.h"
#include <vector>

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
    Texture2D texFlipLeft{};
    Texture2D texFlipRight{};

    Vector2 leftCenter{ 0,0 };
    Vector2 rightCenter{ 0,0 };
    Vector2 leftPivotWorld{ 0,0 };
    Vector2 rightPivotWorld{ 0,0 };
    Vector2 leftLocalPivotPx{ 0,0 };
    Vector2 rightLocalPivotPx{ 0,0 };

    float leftAngleDeg = 24.0f;
    float rightAngleDeg = -24.0f;
    const float leftRestDeg = 24.0f;
    const float leftHitDeg = -50.0f;
    const float rightRestDeg = -24.0f;
    const float rightHitDeg = 50.0f;
    const float flipSpeedDps = 600.0f;

    Vector2 posBall{ 300.0f, 200.0f };

    struct Ball {
        Vector2 position;
        Vector2 velocity;
        bool active;
        float radius;
    };
    std::vector<Ball> balls;

    const float ballSpeed = 300.0f;
    const Vector2 spawnPosition = { 300.0f, 100.0f };
    const float ballRadius = 8.0f; // Radio aproximado de la pelota

    void AdjustWindowToMap();
    void DrawWithPivot(const Texture2D& tex, Vector2 worldPivot, Vector2 localPivotPx, float rotationDeg) const;
    void SpawnBall();
    void UpdateBalls(float dt);
    void DrawBalls();
    void CheckCollisionsWithFlippers(Ball& ball);
    bool CheckCollisionWithFlipper(const Ball& ball, Vector2 pivotWorld, Vector2 localPivot, float angleDeg, bool isRightFlipper);
    Vector2 RotatePoint(Vector2 point, Vector2 pivot, float angleDeg);
    inline bool IsLoaded(const Texture2D& t) const { return t.id != 0; }
};