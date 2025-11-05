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
    //Inicializar sonidos
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

    texBall = LoadTexture("assets/bola.png");
    texMap = LoadTexture("assets/map.png");
    texFlipperLeft = LoadTexture("assets/palanca_inverted.png");
    texFlipperRight = LoadTexture("assets/palanca.png");
    texSpring = LoadTexture("assets/Spring.png");
    textCollectible = LoadTexture("assets/puntos.png");
    texLife = LoadTexture("assets/Vidas.png");

    //Cargar efectos de sonido
    springsound = App->audio->LoadFx("assets/springsound.wav");
    flipersound = App->audio->LoadFx("assets/flipper.wav");
    ballvoid = App->audio->LoadFx("assets/ball_void.wav");
    newball = App->audio->LoadFx("assets/new-ball.wav");
    bonus_fx = App->audio->LoadFx("assets/bonus.wav");

    //Inicializamos la musica de fondo
    App->audio->PlayMusic("assets/background_music.wav");


    // Crear la bola 
    ball = App->physics->CreateCircle(465, 200, 10);
    ball->listener = this;

    // Crear las palas (flippers)
    leftFlipper = App->physics->CreateFlipper(150, 610, texFlipperLeft.width, texFlipperLeft.height, true);
    rightFlipper = App->physics->CreateFlipper(300, 610, texFlipperLeft.width, texFlipperLeft.height, false);

    bumper = App->physics->CreateBumper(235, 64, 25, 1.5f);
    bumper2 = App->physics->CreateBumper(168, 170, 7, 1.5f);
    bumper3 = App->physics->CreateBumper(296, 170, 7, 1.5f);

    spring = App->physics->CreateSpring(465, 360, texSpring.width, texSpring.height); //Ajustar posiciones

    resetZone = App->physics->CreateRectangleSensor(230, 635, 250, 5);

    collectible1 = App->physics->CreateCollectible(68, 62, 10);
    collectible2 = App->physics->CreateCollectible(398, 111, 8);
    collectible3 = App->physics->CreateCollectible(224, 287, 12);

    const int trianglePoints1[] = {
    322, 489,
    320, 521,
    290, 537

    };
    App->physics->CreateTriangularBumper(0, 0, trianglePoints1, 6, 1.5f);

    const int trianglePoints2[] = {
    127, 489,
    127, 521,
    157, 537

    };
    App->physics->CreateTriangularBumper(0, 0, trianglePoints2, 6, 1.5f);
    // Paredes y bordes del tablero
    const int bordeExterior[] = {
        428, 7,
        104, 7,
        64,  40,
        44,  88,
        31,  150,
        31,  164,
        16,  189,
        19,  321,
        25,  340,
        49,  370,
        52,  433,
        32,  458,
        32,  601,
        93,  640,
        354, 640,
        415, 600,
        415, 469,
        400, 454,
        400, 421,
        420, 399,
        428, 368,
        429, 88,
        417, 71,
        407, 69,
        361, 94,
        355, 87,
        394, 52,
        419, 54,
        434, 65,
        448, 90,
        448, 477,
        479, 477,
        479, 71,
        466, 37,
        446, 13,
    };
    
    const int bordeInterior1[] = {
        393, 342,
        393, 148,
        374, 148,
        372, 228,
        364, 251,
        344, 267,
        318, 275,
        296, 275,
        313, 286,
        298, 298,
        376, 352
    };
    
    const int bordeInterior2[] = {
        111, 41,
        87,  62,
        70,  99,
        68,  131,
        85,  143,
        144, 83,
        148, 63
    };
    
    const int bordeInterior3[] = {
        81,192,
        66,227,
        67,295,
        73,319,
        106,345,
        142,309,
        159,302,
        145,295,
        143,278,
        156,271,
        121,259,
        97,228
    };
    
    const int bordeInterior4[] = {
        77, 480,
        77, 577,
        141,613,
        149,599,
        93, 568,
        90, 480,
    };
    
    const int bordeInterior5[] = {
        369, 480,
        369, 577,
        304,613,
        297,599,
        356, 568,
        356, 480
    };
    
    App->physics->CreatePolygonWall(bordeExterior, 68, 6.0f, true);
    App->physics->CreatePolygonWall(bordeInterior1, 22, 6.0f, true);
    App->physics->CreatePolygonWall(bordeInterior2, 14, 6.0f, true);
    App->physics->CreatePolygonWall(bordeInterior3, 24, 6.0f, true);
    App->physics->CreatePolygonWall(bordeInterior4, 12, 6.0f, true);
    App->physics->CreatePolygonWall(bordeInterior5, 12, 6.0f, true);
    

    return true;
}

update_status ModuleGame::Update()
{
    // Velocidad máxima al presionar la tecla
    float flipperSpeed = 15.0f;
    if (IsKeyDown(KEY_LEFT))
    {
        App->physics->MoveFlipper(leftFlipper, -flipperSpeed);
        if (!leftFlipperPressed)
        {
            App->audio->PlayFx(flipersound); // Sonido al presionar
            leftFlipperPressed = true;
        }
    }
    else
    {
        App->physics->MoveFlipper(leftFlipper, flipperSpeed);
        leftFlipperPressed = false;
    }

    // Flipper derecho
    if (IsKeyDown(KEY_RIGHT))
    {
        App->physics->MoveFlipper(rightFlipper, flipperSpeed);
        if (!rightFlipperPressed)
        {
            App->audio->PlayFx(flipersound); // Sonido al presionar
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
        // Comprimir spring
        ((b2PrismaticJoint*)spring->joint)->SetMotorSpeed(5.0f);
        if (!springPressed)
        {
            App->audio->PlayFx(springsound); // Sonido al presionar
            springPressed = true;
        }
    }
    else
    {
        // Soltar spring y lanzar bola
        ((b2PrismaticJoint*)spring->joint)->SetMotorSpeed(-150.0f);
        springPressed = false;
    }

    if (IsKeyPressed(KEY_F1))
    {
        debug = !debug;
    }
    if (IsKeyDown(KEY_F2)) {
        resetBall = true;
    }
    

    if (ball && ball->body)
    {
        float maxSpeed = 15.0f; // m/s
        b2Vec2 vel = ball->body->GetLinearVelocity();
        
        float speed = vel.Length();

        if (speed > maxSpeed)
        {
            vel *= maxSpeed / speed;
            ball->body->SetLinearVelocity(vel);
        }
    }

    // Reiniciar la bola si cae fuera del tablero
    int x, y;
    ball->GetPhysicPosition(x, y);
    if (y > SCREEN_HEIGHT)
    {
        //Sonido cuando bola cae al vacio
        App->physics->DestroyBody(ball);
        ball->listener = this;
        App->audio->PlayFx(ballvoid);
        ball = App->physics->CreateCircle(400, 200, 10);
        App->audio->PlayFx(newball);
        ball->listener = this;
      
    }

    return UPDATE_CONTINUE;
}

update_status ModuleGame::PostUpdate()
{
    if (resetBall) {
        resetBall = false;
        ball->body->SetTransform(b2Vec2(PIXEL_TO_METERS(465), PIXEL_TO_METERS(200)), 0);
        ball->body->SetLinearVelocity(b2Vec2(0, 0));
        ball->body->SetAngularVelocity(0);
    }

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

    //Spring
    int sx, sy;
    spring->GetPhysicPosition(sx, sy);

    float w = (float)texSpring.width;
    float h = (float)texSpring.height;

    // Pivote = centro de la textura
    Vector2 pivot = { w / 2, h / 2 };
    
    // DestRect = posicionar textura sobre el cuerpo físico
    Rectangle dest = {
        (float)sx,
        (float)sy,
        w,
        h
    };

    DrawTexturePro(texSpring,
        { 0, 0, w, h }, // toda la textura
        dest,
        pivot,
        0.0f, // ángulo en grados, para el spring normalmente es vertical
        WHITE
    );

    

    // --- Dibujar collectible 1 ---
    if (collectible1 && collectible1->body && !collectible1->pendingToDelete)
    {
        b2Vec2 pos = collectible1->body->GetPosition();
        
        float angle = collectible1->body->GetAngle() * RAD2DEG;

        float scale = 0.8f; 

        int x = METERS_TO_PIXELS(pos.x);
        int y = METERS_TO_PIXELS(pos.y);

        int drawWidth = (int)(textCollectible.width * scale);
        int drawHeight = (int)(textCollectible.height * scale);

        // dibuja centrado en la posición del cuerpo
        DrawTexturePro(
            textCollectible,
            { 0, 0, (float)textCollectible.width, (float)textCollectible.height }, // rect fuente
            { (float)x, (float)y, (float)drawWidth, (float)drawHeight },           // rect destino
            { drawWidth / 2.0f, drawHeight / 2.0f },                              // pivote (centro)
            0.0f,
            WHITE
        );
    }

    // --- Dibujar collectible 2 ---
    if (collectible2 && collectible2->body && !collectible2->pendingToDelete)
    {
        b2Vec2 pos = collectible2->body->GetPosition();

        float angle = collectible2->body->GetAngle() * RAD2DEG;

        float scale = 0.8f; 

        int x = METERS_TO_PIXELS(pos.x);
        int y = METERS_TO_PIXELS(pos.y);

        int drawWidth = (int)(textCollectible.width * scale);
        int drawHeight = (int)(textCollectible.height * scale);

        // dibuja centrado en la posición del cuerpo
        DrawTexturePro(
            textCollectible,
            { 0, 0, (float)textCollectible.width, (float)textCollectible.height }, // rect fuente
            { (float)x, (float)y, (float)drawWidth, (float)drawHeight },           // rect destino
            { drawWidth / 2.0f, drawHeight / 2.0f },                              // pivote (centro)
            0.0f,
            WHITE
        );
    }

    // --- Dibujar collectible 3 ---
    if (collectible3 && collectible3->body && !collectible3->pendingToDelete)
    {
        b2Vec2 pos = collectible3->body->GetPosition();

        float angle = collectible3->body->GetAngle() * RAD2DEG;

        float scale = 0.8f; 

        int x = METERS_TO_PIXELS(pos.x);
        int y = METERS_TO_PIXELS(pos.y);

        int drawWidth = (int)(textCollectible.width * scale);
        int drawHeight = (int)(textCollectible.height * scale);

        // dibuja centrado en la posición del cuerpo
        DrawTexturePro(
            textCollectible,
            { 0, 0, (float)textCollectible.width, (float)textCollectible.height }, // rect fuente
            { (float)x, (float)y, (float)drawWidth, (float)drawHeight },           // rect destino
            { drawWidth / 2.0f, drawHeight / 2.0f },                              // pivote (centro)
            0.0f,
            WHITE
        );
    }

    if (debug)
        App->physics->DrawDebug(App->renderer);

    // --- Mostrar puntuación en pantalla ---
    DrawText(TextFormat("SCORE: %d", score), 20, 20, 30, WHITE);

    // --- Dibujar vidas debajo del score ---
    if (texLife.id != 0 && lives > 0)

    {
        // Posición base (debajo del score)
        const int baseX = 20;
        const int baseY = 56;   // un poco por debajo del texto del score

        // Tamaño destino (por si el PNG es grande lo escalamos a algo tipo 24–28 px)
        const float iconH = 26.0f;
        const float scale = iconH / (float)texLife.height;
        const float iconW = texLife.width * scale;

        const int spacing = 8;  // separación entre corazones

        for (int i = 0; i < lives; ++i)
        {
            Rectangle src = { 0, 0, (float)texLife.width, (float)texLife.height };
            Rectangle dst = { baseX + i * (iconW + spacing), (float)baseY, iconW, iconH };
            Vector2 origin = { 0.0f, 0.0f };

            DrawTexturePro(texLife, src, dst, origin, 0.0f, WHITE);
        }
    }



    return UPDATE_CONTINUE;
}

bool ModuleGame::CleanUp()
{
    LOG("Cleaning up pinball scene");
    return true;
}

void ModuleGame::OnCollision(PhysBody* bodyA, PhysBody* bodyB)
{
    // sonido genérico de colisión con algo puntuable
    auto ping = [&]() { App->audio->PlayFx(bonus_fx); };

    // Reset si entra en la zona de reinicio
    if ((bodyA == ball && bodyB == resetZone) || (bodyB == ball && bodyA == resetZone))
    {
        resetBall = true;
        return;
    }

    // --- BUMPERS ---
    if ((bodyA == ball && bodyB == bumper) || (bodyB == ball && bodyA == bumper))
    {
        score += POINTS_BUMPER_BIG;
        ping();
    }

    if ((bodyA == ball && bodyB == bumper2) || (bodyB == ball && bodyA == bumper2))
    {
        score += POINTS_BUMPER_SMALL;
        ping();
    }

    if ((bodyA == ball && bodyB == bumper3) || (bodyB == ball && bodyA == bumper3))
    {
        score += POINTS_BUMPER_SMALL;
        ping();
    }

    // --- COLLECTIBLES ---
    if (bodyA == ball && (bodyB == collectible1 || bodyB == collectible2 || bodyB == collectible3))
    {
        bodyB->pendingToDelete = true;   // se elimina en PostUpdate de físicas
        score += POINTS_COLLECTIBLE;
        ping();
    }
    else if (bodyB == ball && (bodyA == collectible1 || bodyA == collectible2 || bodyA == collectible3))
    {
        bodyA->pendingToDelete = true;
        score += POINTS_COLLECTIBLE;
        ping();
    }

}
