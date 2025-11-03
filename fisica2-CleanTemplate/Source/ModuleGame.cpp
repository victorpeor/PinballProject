#include "Globals.h"
#include "Application.h"
#include "ModuleRender.h"
#include "ModulePhysics.h"
#include "ModuleAudio.h"
#include "ModuleGame.h"

ModuleGame::ModuleGame(Application* app, bool start_enabled) : Module(app, start_enabled)
{
    ball = nullptr;
    leftFlipper = nullptr;
    rightFlipper = nullptr;
}

ModuleGame::~ModuleGame() {}

bool ModuleGame::Start()
{
    LOG("Starting Pinball Scene");

    texBall = LoadTexture("assets/bola.png");
    //texMap = LoadTexture("assets/mapa.png");
    texFlipperLeft = LoadTexture("assets/palanca_inverted.png");
    texFlipperRight = LoadTexture("assets/palanca.png");

    // Crear la bola 
    ball = App->physics->CreateCircle(400, 200, 10);
    ball->listener = this;

    // Crear las palas (flippers)
    leftFlipper = App->physics->CreateFlipper(300, 500, texFlipperLeft.width, texFlipperLeft.height, true);
    rightFlipper = App->physics->CreateFlipper(500, 500, texFlipperLeft.width, texFlipperLeft.height, false);

    // Paredes y bordes del tablero
    const int chain_points[] = {
        100, 100, 700, 100, 700, 800, 100, 800
    };
    App->physics->CreateChain(0, 0, chain_points, 8);

    

    return true;
}

update_status ModuleGame::Update()
{
    // Velocidad máxima al presionar la tecla
    float flipperSpeed = 20.0f;
    if (IsKeyDown(KEY_LEFT)) {
        App->physics->MoveFlipper(leftFlipper, -flipperSpeed); // subir
    }
    else {
        App->physics->MoveFlipper(leftFlipper, flipperSpeed);  // bajar
    }

    if (IsKeyDown(KEY_RIGHT)) {
        App->physics->MoveFlipper(rightFlipper, flipperSpeed); // subir
    }
    else {
        App->physics->MoveFlipper(rightFlipper, -flipperSpeed); // bajar
    }

    // Reiniciar la bola si cae fuera del tablero
    int x, y;
    ball->GetPhysicPosition(x, y);
    if (y > SCREEN_HEIGHT)
    {
        App->physics->DestroyBody(ball);
        ball = App->physics->CreateCircle(400, 200, 10);
        ball->listener = this;
    }

    return UPDATE_CONTINUE;
}

update_status ModuleGame::PostUpdate()
{
    // --- Mapa y bola ---
    DrawTexture(texMap, 0, 0, WHITE);

    int bx, by;
    ball->GetPhysicPosition(bx, by);
    DrawTexture(texBall, bx - texBall.width / 2, by - texBall.height / 2, WHITE);

    // --- Flipper izquierdo ---
    int x, y;
    leftFlipper->GetPhysicPosition(x, y);          // centro físico
    float angleLeft = leftFlipper->GetRotation() * RAD2DEG;

    // Dimensiones físicas del flipper
    float wL = (float)leftFlipper->width;
    float hL = (float)leftFlipper->height;

    // Pivote de rotación = extremo izquierdo del flipper, centro vertical
    Vector2 pivotLeft = { wL / 2, hL / 2 };

    // DestRect = coloca la textura de forma que el pivote coincida con la posición física
    Rectangle destLeft = {
        (float)x ,  // restamos wL/2 porque GetPhysicPosition() da el centro de la pala
        (float)y ,
        wL,
        hL
    };

    DrawTexturePro(texFlipperLeft,
        { 0, 0, (float)texFlipperLeft.width, (float)texFlipperLeft.height },
        destLeft,
        pivotLeft,
        angleLeft,
        WHITE);

    
    // Flipper derecho
    rightFlipper->GetPhysicPosition(x, y);           // centro físico
    float angleRight = rightFlipper->GetRotation() * RAD2DEG; // usar tal cual

    // Dimensiones físicas del flipper en píxeles
    float wR = (float)rightFlipper->width;
    float hR = (float)rightFlipper->height;

    // Pivote = extremo derecho del flipper, centro vertical
    Vector2 pivotRight = { wR / 2, hR / 2 };

    // DestRect = colocar la textura de modo que el pivote coincida con la posición física
    Rectangle destRight = {
        (float)x,  // simplemente centro físico menos pivot.x
        (float)y,
        wR,
        hR
    };

    DrawTexturePro(texFlipperRight,
        { 0, 0, (float)texFlipperRight.width, (float)texFlipperRight.height },
        destRight,
        pivotRight,
        angleRight,
        WHITE);

    return UPDATE_CONTINUE;
}

bool ModuleGame::CleanUp()
{
    LOG("Cleaning up pinball scene");
    return true;
}

void ModuleGame::OnCollision(PhysBody* bodyA, PhysBody* bodyB)
{
    // Ejemplo: reproducir sonido o sumar puntos
    App->audio->PlayFx(bonus_fx);
}