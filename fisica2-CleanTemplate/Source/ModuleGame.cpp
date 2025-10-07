#include "Application.h"
#include "ModuleGame.h"
#include "Globals.h"

ModuleGame::ModuleGame(Application* app, bool start_enabled) : Module(app, start_enabled) {}
ModuleGame::~ModuleGame() {}

bool ModuleGame::Start()
{
    texMap = LoadTexture("Assets/map.png");
    texBall = LoadTexture("Assets/bola.png");
    texFlipLeft = LoadTexture("Assets/palanca_inverted.png");
    texFlipRight = LoadTexture("Assets/palanca.png");
    if (!IsLoaded(texMap) || !IsLoaded(texBall) || !IsLoaded(texFlipLeft) || !IsLoaded(texFlipRight))
        return false;

    AdjustWindowToMap();

    const float offsetY = 8.0f;
    leftCenter = { 210.0f - texFlipLeft.width * 0.5f, 604.0f + texFlipLeft.height * 0.5f + offsetY };
    rightCenter = { 298.0f - texFlipRight.width * 0.5f, 604.0f + texFlipRight.height * 0.5f + offsetY };

    leftLocalPivotPx = { (float)texFlipLeft.width * 0.033333f, (float)texFlipLeft.height * 0.55f };   // punta interna
    rightLocalPivotPx = { (float)texFlipRight.width * 0.966667f, (float)texFlipRight.height * 0.55f };  // punta externa

    leftPivotWorld = {
        leftCenter.x - (float)texFlipLeft.width * 0.5f + leftLocalPivotPx.x,
        leftCenter.y - (float)texFlipLeft.height * 0.5f + leftLocalPivotPx.y
    };
    rightPivotWorld = {
        rightCenter.x - (float)texFlipRight.width * 0.5f + rightLocalPivotPx.x,
        rightCenter.y - (float)texFlipRight.height * 0.5f + rightLocalPivotPx.y
    };

    // Ajuste fino DERECHA: un poco hacia el centro (izquierda) y abajo (diagonal)
    rightPivotWorld.x += 64.0f; // ↓ 4 px respecto a 68 → más separada de la pared
    rightPivotWorld.y += 1.0f;  // ↓ 4 px respecto a -3 → ligeramente más baja

    leftAngleDeg = leftRestDeg;
    rightAngleDeg = rightRestDeg;
    return true;
}

update_status ModuleGame::Update()
{
    const float dt = GetFrameTime();
    const float step = flipSpeedDps * dt;

    const float leftTarget = IsKeyDown(KEY_LEFT) ? leftHitDeg : leftRestDeg;
    const float rightTarget = IsKeyDown(KEY_RIGHT) ? rightHitDeg : rightRestDeg;

    auto approach = [](float cur, float tgt, float s) {
        if (cur < tgt) { cur += s; if (cur > tgt) cur = tgt; }
        else if (cur > tgt) { cur -= s; if (cur < tgt) cur = tgt; }
        return cur;
        };

    leftAngleDeg = approach(leftAngleDeg, leftTarget, step);
    rightAngleDeg = approach(rightAngleDeg, rightTarget, step);

    ClearBackground(Color{ 20,20,20,255 });
    DrawTexture(texMap, 0, 0, WHITE);

    DrawWithPivot(texFlipLeft, leftPivotWorld, leftLocalPivotPx, leftAngleDeg);
    DrawWithPivot(texFlipRight, rightPivotWorld, rightLocalPivotPx, rightAngleDeg);

    DrawTexture(texBall, (int)posBall.x, (int)posBall.y, WHITE);
    return update_status::UPDATE_CONTINUE;
}

bool ModuleGame::CleanUp()
{
    if (IsLoaded(texMap))       UnloadTexture(texMap);
    if (IsLoaded(texBall))      UnloadTexture(texBall);
    if (IsLoaded(texFlipLeft))  UnloadTexture(texFlipLeft);
    if (IsLoaded(texFlipRight)) UnloadTexture(texFlipRight);

    texMap = texBall = texFlipLeft = texFlipRight = Texture2D{};
    return true;
}

void ModuleGame::AdjustWindowToMap()
{
    SetWindowMinSize(texMap.width, texMap.height);
    SetWindowSize(texMap.width, texMap.height);
}

void ModuleGame::DrawWithPivot(const Texture2D& tex, Vector2 worldPivot, Vector2 localPivotPx, float rotationDeg) const
{
    Rectangle src{ 0,0,(float)tex.width,(float)tex.height };
    Rectangle dst{
        worldPivot.x - localPivotPx.x,
        worldPivot.y - localPivotPx.y,
        (float)tex.width,
        (float)tex.height
    };
    Vector2 origin{ localPivotPx.x, localPivotPx.y };
    DrawTexturePro(tex, src, dst, origin, rotationDeg, WHITE);
}
