#include "Application.h"
#include "ModuleGame.h"
#include "Globals.h"

ModuleGame::ModuleGame(Application* app, bool start_enabled)
    : Module(app, start_enabled)
{
}

ModuleGame::~ModuleGame() {}

bool ModuleGame::Start()
{
    texMap = LoadTexture("Assets/map.png");
    texBall = LoadTexture("Assets/bola.png");
    texFlipLeft = LoadTexture("Assets/palanca_inverted.png"); // IZQUIERDA
    texFlipRight = LoadTexture("Assets/palanca.png");          // DERECHA

    if (!IsLoaded(texMap) || !IsLoaded(texBall) || !IsLoaded(texFlipLeft) || !IsLoaded(texFlipRight))
    {
        LOG("Error cargando texturas desde Assets/");
        return false;
    }

    AdjustWindowToMap();

    // Posición base de los compañeros: (210 - w/2, 604 + h/2) y (298 - w/2, 604 + h/2)
    // Ajuste: +8 px hacia abajo (Y) y -6 px hacia atrás (X)
    const float offsetY = 8.0f;
    const float offsetX = -6.0f;

    leftCenter = { 210.0f - texFlipLeft.width * 0.5f + offsetX,
                    604.0f + texFlipLeft.height * 0.5f + offsetY };

    rightCenter = { 298.0f - texFlipRight.width * 0.5f - offsetX,
                    604.0f + texFlipRight.height * 0.5f + offsetY };

    leftAngleDeg = 24.0f;
    rightAngleDeg = -24.0f;

    return true;
}

update_status ModuleGame::Update()
{
    ClearBackground(Color{ 20, 20, 20, 255 });
    DrawTexture(texMap, 0, 0, WHITE);

    DrawCentered(texFlipLeft, leftCenter, leftAngleDeg);
    DrawCentered(texFlipRight, rightCenter, rightAngleDeg);

    DrawTexture(texBall, (int)posBall.x, (int)posBall.y, WHITE);

    return update_status::UPDATE_CONTINUE;
}

bool ModuleGame::CleanUp()
{
    if (IsLoaded(texMap))       UnloadTexture(texMap);
    if (IsLoaded(texBall))      UnloadTexture(texBall);
    if (IsLoaded(texFlipLeft))  UnloadTexture(texFlipLeft);
    if (IsLoaded(texFlipRight)) UnloadTexture(texFlipRight);

    texMap = Texture2D{};
    texBall = Texture2D{};
    texFlipLeft = Texture2D{};
    texFlipRight = Texture2D{};
    return true;
}

void ModuleGame::AdjustWindowToMap()
{
    SetWindowMinSize(texMap.width, texMap.height);
    SetWindowSize(texMap.width, texMap.height);
}

void ModuleGame::DrawCentered(Texture2D& tex, Vector2 center, float rotationDeg)
{
    Rectangle src{ 0.0f, 0.0f, (float)tex.width, (float)tex.height };
    Rectangle dst{ center.x, center.y, (float)tex.width, (float)tex.height };
    Vector2   origin{ (float)tex.width * 0.5f, (float)tex.height * 0.5f };
    DrawTexturePro(tex, src, dst, origin, rotationDeg, WHITE);
}
