#include "Application.h"
#include "ModuleGame.h"
#include "Globals.h"

ModuleGame::ModuleGame(Application* app, bool start_enabled) : Module(app, start_enabled) {}
ModuleGame::~ModuleGame() {}

bool ModuleGame::Start()
{
    // Carga (OJO: lado correcto, igual que tus compis)
    texMap = LoadTexture("Assets/map.png");
    texBall = LoadTexture("Assets/bola.png");
    texFlipLeft = LoadTexture("Assets/palanca_inverted.png"); // IZQ
    texFlipRight = LoadTexture("Assets/palanca.png");          // DER

    if (!IsLoaded(texMap) || !IsLoaded(texBall) || !IsLoaded(texFlipLeft) || !IsLoaded(texFlipRight))
    {
        LOG("Error cargando texturas de Assets/");
        return false;
    }

    // Ventana exactamente al mapa (sin bordes)
    AdjustWindowToMap();

    // === Centros EXACTOS como en su proyecto (top-left -> center) ===
    // Box(App->physics, 210 - w/2, 604 + h/2, w,h, ...) ==> center = (x, y)
    flipLeftCenter = { 210.0f - texFlipLeft.width * 0.5f, 604.0f + texFlipLeft.height * 0.5f };
    flipRightCenter = { 298.0f - texFlipRight.width * 0.5f, 604.0f + texFlipRight.height * 0.5f };

    // Arrancamos sin giro; si luego quieres “reposo” suave, pon ±8.0f
    flipLeftAngle = 0.0f;
    flipRightAngle = 0.0f;

    return true;
}

update_status ModuleGame::Update()
{
    ClearBackground(Color{ 20,20,20,255 });

    // Mapa
    DrawTexture(texMap, 0, 0, WHITE);

    // Palancas como en el render de sus Box::Update() (centradas + rotación)
    DrawCentered(texFlipLeft, flipLeftCenter, flipLeftAngle);
    DrawCentered(texFlipRight, flipRightCenter, flipRightAngle);

    // Bola (referencia)
    DrawTexture(texBall, (int)posBall.x, (int)posBall.y, WHITE);

    return update_status::UPDATE_CONTINUE;
}

bool ModuleGame::CleanUp()
{
    if (IsLoaded(texMap))       UnloadTexture(texMap);
    if (IsLoaded(texBall))      UnloadTexture(texBall);
    if (IsLoaded(texFlipLeft))  UnloadTexture(texFlipLeft);
    if (IsLoaded(texFlipRight)) UnloadTexture(texFlipRight);
    return true;
}

void ModuleGame::AdjustWindowToMap()
{
    SetWindowMinSize(texMap.width, texMap.height);
    SetWindowSize(texMap.width, texMap.height);
}

// Dibujo “estilo Box::Update()” de tus compis: origen en centro del sprite
void ModuleGame::DrawCentered(Texture2D& tex, Vector2 center, float rotationDeg)
{
    Rectangle src{ 0,0,(float)tex.width,(float)tex.height };
    Rectangle dst{ center.x, center.y, (float)tex.width, (float)tex.height };
    Vector2   origin{ (float)tex.width * 0.5f, (float)tex.height * 0.5f };
    DrawTexturePro(tex, src, dst, origin, rotationDeg, WHITE);
}
