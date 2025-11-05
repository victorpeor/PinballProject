#include "Globals.h"
#include "Application.h"
#include "ModuleRender.h"
#include "ModulePhysics.h"
#include "ModuleAudio.h"
#include "ModuleGame.h"
#include "raylib.h"

ModuleGame::ModuleGame(Application* app, bool start_enabled) : Module(app, start_enabled)
{
    ball = nullptr;
    leftFlipper = nullptr;
    rightFlipper = nullptr;

    springsound = 0;
    flipersound = 0;
    ballvoid = 0;
    newball = 0;
    bonus_fx = 0;
    background_music = { 0 };
}

ModuleGame::~ModuleGame() {}

bool ModuleGame::Start()
{
    LOG("Starting Pinball Scene");

    // Texturas
    texBall = LoadTexture("assets/bola.png");
    texMap = LoadTexture("assets/map.png");
    texFlipperLeft = LoadTexture("assets/palanca_inverted.png");
    texFlipperRight = LoadTexture("assets/palanca.png");
    texSpring = LoadTexture("assets/Spring.png");
    textCollectible = LoadTexture("assets/puntos.png");
    texLife = LoadTexture("assets/Vidas.png");
    texLose = LoadTexture("assets/loose.png");   // pantalla de derrota
    texMenu = LoadTexture("assets/menu.png");    // pantalla de menú

    // Sonidos
    springsound = App->audio->LoadFx("assets/springsound.wav");
    flipersound = App->audio->LoadFx("assets/flipper.wav");
    ballvoid = App->audio->LoadFx("assets/ball_void.wav");
    newball = App->audio->LoadFx("assets/new-ball.wav");
    bonus_fx = App->audio->LoadFx("assets/bonus.wav");

    // Música de fondo del menú
    App->audio->PlayMusic("assets/background_music.wav");

    // Crear palancas y resto de escenario (persisten entre estados)
    leftFlipper = App->physics->CreateFlipper(150, 610, texFlipperLeft.width, texFlipperLeft.height, true);
    rightFlipper = App->physics->CreateFlipper(300, 610, texFlipperLeft.width, texFlipperLeft.height, false);

    bumper = App->physics->CreateBumper(235, 64, 25, 1.5f);
    bumper2 = App->physics->CreateBumper(168, 170, 7, 1.5f);
    bumper3 = App->physics->CreateBumper(296, 170, 7, 1.5f);

    spring = App->physics->CreateSpring(465, 360, texSpring.width, texSpring.height);
    resetZone = App->physics->CreateRectangleSensor(230, 635, 250, 5);

    collectible1 = App->physics->CreateCollectible(68, 62, 10);
    collectible2 = App->physics->CreateCollectible(398, 111, 8);
    collectible3 = App->physics->CreateCollectible(224, 287, 12);

    // Bordes y bumpers triangulares
    const int trianglePoints1[] = { 322, 489, 320, 521, 290, 537 };
    App->physics->CreateTriangularBumper(0, 0, trianglePoints1, 6, 1.5f);

    const int trianglePoints2[] = { 127, 489, 127, 521, 157, 537 };
    App->physics->CreateTriangularBumper(0, 0, trianglePoints2, 6, 1.5f);

    const int bordeExterior[] = {
        428, 7, 104, 7, 64, 40, 44, 88, 31, 150, 31, 164,
        16, 189, 19, 321, 25, 340, 49, 370, 52, 433, 32, 458,
        32, 601, 93, 640, 354, 640, 415, 600, 415, 469, 400, 454,
        400, 421, 420, 399, 428, 368, 429, 88, 417, 71, 407, 69,
        361, 94, 355, 87, 394, 52, 419, 54, 434, 65, 448, 90,
        448, 477, 479, 477, 479, 71, 466, 37, 446, 13,
    };

    const int bordeInterior1[] = {
        393, 342, 393, 148, 374, 148, 372, 228, 364, 251,
        344, 267, 318, 275, 296, 275, 313, 286, 298, 298, 376, 352
    };

    const int bordeInterior2[] = {
        111, 41, 87, 62, 70, 99, 68, 131, 85, 143, 144, 83, 148, 63
    };

    const int bordeInterior3[] = {
        81,192,66,227,67,295,73,319,106,345,142,309,
        159,302,145,295,143,278,156,271,121,259,97,228
    };

    const int bordeInterior4[] = {
        77,480,77,577,141,613,149,599,93,568,90,480
    };

    const int bordeInterior5[] = {
        369,480,369,577,304,613,297,599,356,568,356,480
    };

    App->physics->CreatePolygonWall(bordeExterior, 68, 6.0f, true);
    App->physics->CreatePolygonWall(bordeInterior1, 22, 6.0f, true);
    App->physics->CreatePolygonWall(bordeInterior2, 14, 6.0f, true);
    App->physics->CreatePolygonWall(bordeInterior3, 24, 6.0f, true);
    App->physics->CreatePolygonWall(bordeInterior4, 12, 6.0f, true);
    App->physics->CreatePolygonWall(bordeInterior5, 12, 6.0f, true);

    // Estado inicial
    currentState = GameState::MENU;
    nextState = currentState;
    fading = false;
    fadeAlpha = 0.0f;

    return true;
}

void ModuleGame::StartTransitionTo(GameState target)
{
    if (fading) return; // ya en transición
    nextState = target;
    fadeOut = true;
    fadeIn = false;
    fading = true;
    // si quieres acelerar/ralentizar por transición, ajusta fadeSpeed aquí
}

void ModuleGame::ApplyStateChangeAtBlack()
{
    // Pantalla ya está en negro (alpha==1). Cambiamos estado y ejecutamos EnterState.
    currentState = nextState;
    EnterState(currentState);

    // Cambiamos a fade-in
    fadeOut = false;
    fadeIn = true;
}

void ModuleGame::EnterState(GameState s)
{
    switch (s)
    {
    case GameState::MENU:
        // Volver a menú: destruir la bola, reiniciar info y música del menú
        if (ball) { App->physics->DestroyBody(ball); ball = nullptr; }
        score = 0;
        lives = 3;
        resetBall = false;
        loseLifePending = false;
        leftFlipperPressed = rightFlipperPressed = springPressed = false;
        App->audio->PlayMusic("assets/background_music.wav");
        break;

    case GameState::PLAYING:
        // Entrar a jugar: crear bola y parar música de menú
        if (ball) { App->physics->DestroyBody(ball); ball = nullptr; }
        ball = App->physics->CreateCircle(465, 200, 10);
        if (ball) ball->listener = this;

        score = 0;
        lives = 3;
        loseLifePending = false;
        resetBall = false;
        leftFlipperPressed = rightFlipperPressed = springPressed = false;

        App->audio->StopMusic();
        break;

    case GameState::GAMEOVER:
        // Entrar a game over: destruir bola y dejar pantalla de derrota
        if (ball) { App->physics->DestroyBody(ball); ball = nullptr; }
        break;
    }
}

update_status ModuleGame::Update()
{
    // --- INPUT por estado ---
    if (currentState == GameState::MENU)
    {
        if (IsKeyPressed(KEY_SPACE))
        {
            StartTransitionTo(GameState::PLAYING);
        }
    }
    else if (currentState == GameState::GAMEOVER)
    {
        // Regresar al menú con espacio
        if (IsKeyPressed(KEY_SPACE))
        {
            // Guardar récord antes de cambiar
            if (score > highScore) highScore = score;
            StartTransitionTo(GameState::MENU);
        }
    }
    else if (currentState == GameState::PLAYING)
    {
        // Movimiento de flippers
        float flipperSpeed = 15.0f;

        if (IsKeyDown(KEY_LEFT))
        {
            App->physics->MoveFlipper(leftFlipper, -flipperSpeed);
            if (!leftFlipperPressed)
            {
                App->audio->PlayFx(flipersound);
                leftFlipperPressed = true;
            }
        }
        else
        {
            App->physics->MoveFlipper(leftFlipper, flipperSpeed);
            leftFlipperPressed = false;
        }

        if (IsKeyDown(KEY_RIGHT))
        {
            App->physics->MoveFlipper(rightFlipper, flipperSpeed);
            if (!rightFlipperPressed)
            {
                App->audio->PlayFx(flipersound);
                rightFlipperPressed = true;
            }
        }
        else
        {
            App->physics->MoveFlipper(rightFlipper, -flipperSpeed);
            rightFlipperPressed = false;
        }

        if (IsKeyDown(KEY_DOWN))
        {
            ((b2PrismaticJoint*)spring->joint)->SetMotorSpeed(5.0f);
            if (!springPressed)
            {
                App->audio->PlayFx(springsound);
                springPressed = true;
            }
        }
        else
        {
            ((b2PrismaticJoint*)spring->joint)->SetMotorSpeed(-150.0f);
            springPressed = false;
        }

        if (IsKeyPressed(KEY_F1)) debug = !debug;
        if (IsKeyDown(KEY_F2)) resetBall = true;

        if (ball && ball->body)
        {
            float maxSpeed = 15.0f;
            b2Vec2 vel = ball->body->GetLinearVelocity();
            float speed = vel.Length();
            if (speed > maxSpeed)
            {
                vel *= maxSpeed / speed;
                ball->body->SetLinearVelocity(vel);
            }
        }

        // Reiniciar o perder vida si cae fuera
        if (ball)
        {
            int x, y;
            ball->GetPhysicPosition(x, y);
            if (y > SCREEN_HEIGHT)
            {
                if (lives > 0) lives--;

                if (lives <= 0)
                {
                    // Transición a GAMEOVER
                    StartTransitionTo(GameState::GAMEOVER);
                }
                else
                {
                    App->physics->DestroyBody(ball);
                    App->audio->PlayFx(ballvoid);
                    ball = App->physics->CreateCircle(400, 200, 10);
                    App->audio->PlayFx(newball);
                    if (ball) ball->listener = this;
                }
            }
        }
    }

    // --- Gestión del fade (se evalúa siempre) ---
    if (fading)
    {
        if (fadeOut)
        {
            fadeAlpha += fadeSpeed;
            if (fadeAlpha >= 1.0f)
            {
                fadeAlpha = 1.0f;
                ApplyStateChangeAtBlack(); // cambia de estado con pantalla negra
            }
        }
        else if (fadeIn)
        {
            fadeAlpha -= fadeSpeed;
            if (fadeAlpha <= 0.0f)
            {
                fadeAlpha = 0.0f;
                fadeIn = false;
                fading = false;
            }
        }
    }

    return UPDATE_CONTINUE;
}

void ModuleGame::DrawMenu()
{
    Rectangle src = { 0, 0, (float)texMenu.width, (float)texMenu.height };
    Rectangle dst = { 0, 0, (float)SCREEN_WIDTH, (float)SCREEN_HEIGHT };
    Vector2 origin = { 0, 0 };
    DrawTexturePro(texMenu, src, dst, origin, 0.0f, WHITE);
    // Opcional: DrawText("Pulsa ENTER para empezar", 70, 560, 20, WHITE);
}

void ModuleGame::DrawGame()
{
    if (resetBall)
    {
        resetBall = false;
        if (ball && ball->body)
        {
            ball->body->SetTransform(b2Vec2(PIXEL_TO_METERS(465), PIXEL_TO_METERS(200)), 0);
            ball->body->SetLinearVelocity(b2Vec2(0, 0));
            ball->body->SetAngularVelocity(0);
        }
    }

    if (loseLifePending)
    {
        if (lives > 0) lives--;
        loseLifePending = false;

        if (lives <= 0)
        {
            StartTransitionTo(GameState::GAMEOVER);
        }
        else
        {
            App->audio->PlayFx(newball);
        }
    }

    // Mapa
    DrawTexture(texMap, 0, 0, WHITE);

    // Bola
    if (ball)
    {
        int bx, by;
        ball->GetPhysicPosition(bx, by);
        DrawTexture(texBall, bx - texBall.width / 2, by - texBall.height / 2, WHITE);
    }

    // Flipper izquierdo
    int x, y;
    leftFlipper->GetPhysicPosition(x, y);
    float angleLeft = leftFlipper->GetRotation() * RAD2DEG;
    float wL = (float)leftFlipper->width;
    float hL = (float)leftFlipper->height;
    Vector2 pivotLeft = { wL / 2, hL / 2 };
    Rectangle destLeft = { (float)x, (float)y, wL, hL };
    DrawTexturePro(texFlipperLeft,
        { 0, 0, (float)texFlipperLeft.width, (float)texFlipperLeft.height },
        destLeft, pivotLeft, angleLeft, WHITE);

    // Flipper derecho
    rightFlipper->GetPhysicPosition(x, y);
    float angleRight = rightFlipper->GetRotation() * RAD2DEG;
    float wR = (float)rightFlipper->width;
    float hR = (float)rightFlipper->height;
    Vector2 pivotRight = { wR / 2, hR / 2 };
    Rectangle destRight = { (float)x, (float)y, wR, hR };
    DrawTexturePro(texFlipperRight,
        { 0, 0, (float)texFlipperRight.width, (float)texFlipperRight.height },
        destRight, pivotRight, angleRight, WHITE);

    // Spring
    int sx, sy;
    spring->GetPhysicPosition(sx, sy);
    float w = (float)texSpring.width;
    float h = (float)texSpring.height;
    Vector2 pivot = { w / 2, h / 2 };
    Rectangle dest = { (float)sx, (float)sy, w, h };
    DrawTexturePro(texSpring, { 0, 0, w, h }, dest, pivot, 0.0f, WHITE);

    // Collectibles
    auto drawCollectible = [&](PhysBody* cbody)
        {
            if (cbody && cbody->body && !cbody->pendingToDelete)
            {
                b2Vec2 pos = cbody->body->GetPosition();
                int cx = METERS_TO_PIXELS(pos.x);
                int cy = METERS_TO_PIXELS(pos.y);
                float scale = 0.8f;
                int drawWidth = (int)(textCollectible.width * scale);
                int drawHeight = (int)(textCollectible.height * scale);
                DrawTexturePro(textCollectible,
                    { 0,0,(float)textCollectible.width,(float)textCollectible.height },
                    { (float)cx,(float)cy,(float)drawWidth,(float)drawHeight },
                    { drawWidth / 2.0f, drawHeight / 2.0f },
                    0.0f, WHITE);
            }
        };

    drawCollectible(collectible1);
    drawCollectible(collectible2);
    drawCollectible(collectible3);

    if (debug) App->physics->DrawDebug(App->renderer);

    // Score y vidas
    DrawText(TextFormat("SCORE: %d", score), 20, 20, 30, WHITE);
    DrawText(TextFormat("HIGH SCORE: %d", highScore), 20, 50, 25, WHITE);

    if (texLife.id != 0 && lives > 0)
    {
        const int baseX = 20;
        const int baseY = 56;
        const float iconH = 26.0f;
        const float scale = iconH / (float)texLife.height;
        const float iconW = texLife.width * scale;
        const int spacing = 8;

        for (int i = 0; i < lives; ++i)
        {
            Rectangle src = { 0,0,(float)texLife.width,(float)texLife.height };
            Rectangle dst = { baseX + i * (iconW + spacing),(float)baseY,iconW,iconH };
            Vector2 origin = { 0.0f,0.0f };
            DrawTexturePro(texLife, src, dst, origin, 0.0f, WHITE);
        }
    }
}

void ModuleGame::DrawGameOver()
{
    Rectangle src = { 0, 0, (float)texLose.width, (float)texLose.height };
    Rectangle dst = { 0, 0, (float)SCREEN_WIDTH, (float)SCREEN_HEIGHT };
    Vector2 origin = { 0, 0 };
    DrawTexturePro(texLose, src, dst, origin, 0.0f, WHITE);
}

update_status ModuleGame::PostUpdate()
{
    // Dibujo por estado
    switch (currentState)
    {
    case GameState::MENU:
        DrawMenu();
        break;
    case GameState::PLAYING:
        DrawGame();
        break;
    case GameState::GAMEOVER:
        DrawGameOver();
        break;
    }

    // Overlay de fade si procede
    if (fading)
    {
        DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Fade(BLACK, fadeAlpha));
    }

    return UPDATE_CONTINUE;
}

bool ModuleGame::CleanUp()
{
    LOG("Cleaning up pinball scene");

    // Texturas (descarga segura si fueron cargadas)
    if (texLose.id != 0) UnloadTexture(texLose);
    if (texMenu.id != 0) UnloadTexture(texMenu);
    if (texBall.id != 0) UnloadTexture(texBall);
    if (texMap.id != 0) UnloadTexture(texMap);
    if (texFlipperLeft.id != 0) UnloadTexture(texFlipperLeft);
    if (texFlipperRight.id != 0) UnloadTexture(texFlipperRight);
    if (texSpring.id != 0) UnloadTexture(texSpring);
    if (textCollectible.id != 0) UnloadTexture(textCollectible);
    if (texLife.id != 0) UnloadTexture(texLife);

    return true;
}

void ModuleGame::OnCollision(PhysBody* bodyA, PhysBody* bodyB)
{
    auto ping = [&]() { App->audio->PlayFx(bonus_fx); };

    if ((bodyA == ball && bodyB == resetZone) || (bodyB == ball && bodyA == resetZone))
    {
        if (!resetBall)
        {
            resetBall = true;
            loseLifePending = true;
            App->audio->PlayFx(ballvoid);
        }
        return;
    }

    if ((bodyA == ball && bodyB == bumper) || (bodyB == ball && bodyA == bumper))
    {
        score += POINTS_BUMPER_BIG; ping();
    }

    if ((bodyA == ball && bodyB == bumper2) || (bodyB == ball && bodyA == bumper2))
    {
        score += POINTS_BUMPER_SMALL; ping();
    }

    if ((bodyA == ball && bodyB == bumper3) || (bodyB == ball && bodyA == bumper3))
    {
        score += POINTS_BUMPER_SMALL; ping();
    }

    if (bodyA == ball && (bodyB == collectible1 || bodyB == collectible2 || bodyB == collectible3))
    {
        bodyB->pendingToDelete = true; score += POINTS_COLLECTIBLE; ping();
    }
    else if (bodyB == ball && (bodyA == collectible1 || bodyA == collectible2 || bodyA == collectible3))
    {
        bodyA->pendingToDelete = true; score += POINTS_COLLECTIBLE; ping();
    }
}
