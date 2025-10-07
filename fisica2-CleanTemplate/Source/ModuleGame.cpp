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

    // Inicializar el vector de pelotas
    balls.clear();

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

    // Detectar tecla 1 para spawnear pelota
    if (IsKeyPressed(KEY_ONE)) {
        SpawnBall();
    }

    // Actualizar física de las pelotas
    UpdateBalls(dt);

    // Dibujar
    ClearBackground(Color{ 20,20,20,255 });
    DrawTexture(texMap, 0, 0, WHITE);

    DrawWithPivot(texFlipLeft, leftPivotWorld, leftLocalPivotPx, leftAngleDeg);
    DrawWithPivot(texFlipRight, rightPivotWorld, rightLocalPivotPx, rightAngleDeg);

    // Dibujar todas las pelotas
    DrawBalls();

    return update_status::UPDATE_CONTINUE;
}

bool ModuleGame::CleanUp()
{
    if (IsLoaded(texMap))       UnloadTexture(texMap);
    if (IsLoaded(texBall))      UnloadTexture(texBall);
    if (IsLoaded(texFlipLeft))  UnloadTexture(texFlipLeft);
    if (IsLoaded(texFlipRight)) UnloadTexture(texFlipRight);

    texMap = texBall = texFlipLeft = texFlipRight = Texture2D{};
    balls.clear();
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

void ModuleGame::SpawnBall()
{
    Ball newBall;
    newBall.position = spawnPosition;

    // Velocidad inicial aleatoria hacia abajo con un poco de variación horizontal
    newBall.velocity = { (float)(GetRandomValue(-50, 50)), ballSpeed };
    newBall.active = true;

    balls.push_back(newBall);

    // Opcional: Limitar el número máximo de pelotas para evitar lag
    if (balls.size() > 20) {
        balls.erase(balls.begin()); // Eliminar la pelota más antigua
    }
}

void ModuleGame::UpdateBalls(float dt)
{
    for (auto& ball : balls) {
        if (!ball.active) continue;

        // Actualizar posición
        ball.position.x += ball.velocity.x * dt;
        ball.position.y += ball.velocity.y * dt;

        // Simular gravedad
        ball.velocity.y += 500.0f * dt;

        // Colisiones simples con los bordes de la pantalla
        if (ball.position.x <= 0) {
            ball.position.x = 0;
            ball.velocity.x = -ball.velocity.x * 0.8f; // Rebote con pérdida de energía
        }
        else if (ball.position.x >= texMap.width - texBall.width) {
            ball.position.x = texMap.width - texBall.width;
            ball.velocity.x = -ball.velocity.x * 0.8f;
        }

        if (ball.position.y <= 0) {
            ball.position.y = 0;
            ball.velocity.y = -ball.velocity.y * 0.8f;
        }
        else if (ball.position.y >= texMap.height - texBall.height) {
            ball.position.y = texMap.height - texBall.height;
            ball.velocity.y = -ball.velocity.y * 0.8f;

            // Si la pelota está muy quieta en el fondo, desactivarla
            if (fabs(ball.velocity.y) < 50.0f && fabs(ball.velocity.x) < 10.0f) {
                ball.active = false;
            }
        }

        // Limitar velocidad máxima
        float speed = sqrtf(ball.velocity.x * ball.velocity.x + ball.velocity.y * ball.velocity.y);
        float maxSpeed = 800.0f;
        if (speed > maxSpeed) {
            ball.velocity.x = (ball.velocity.x / speed) * maxSpeed;
            ball.velocity.y = (ball.velocity.y / speed) * maxSpeed;
        }
    }

    // Eliminar pelotas inactivas (opcional, para limpiar memoria)
    balls.erase(std::remove_if(balls.begin(), balls.end(),
        [](const Ball& ball) { return !ball.active; }),
        balls.end());
}

void ModuleGame::DrawBalls()
{
    for (const auto& ball : balls) {
        if (ball.active) {
            DrawTexture(texBall, (int)ball.position.x, (int)ball.position.y, WHITE);
        }
    }

    // También dibujar la pelota original si aún la quieres mantener
    DrawTexture(texBall, (int)posBall.x, (int)posBall.y, WHITE);
}
