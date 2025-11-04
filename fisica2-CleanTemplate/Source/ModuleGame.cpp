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
    texMap = LoadTexture("assets/map.png");
    texFlipperLeft = LoadTexture("assets/palanca_inverted.png");
    texFlipperRight = LoadTexture("assets/palanca.png");
    texSpring = LoadTexture("assets/Spring.png");
    // Crear la bola 
    ball = App->physics->CreateCircle(465, 200, 10);
    ball->listener = this;

    // Crear las palas (flippers)
    leftFlipper = App->physics->CreateFlipper(150, 610, texFlipperLeft.width, texFlipperLeft.height, true);
    rightFlipper = App->physics->CreateFlipper(300, 610, texFlipperLeft.width, texFlipperLeft.height, false);

    spring = App->physics->CreateSpring(465, 380, texSpring.width, texSpring.height); //Ajustar posiciones

    resetZone = App->physics->CreateRectangleSensor(230, 630, 250, 5);
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
        85,  60,
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
        77, 513,
        77, 577,
        141,613,
        149,599,
        93, 568,
        90, 512


    };
    
    const int bordeInterior5[] = {
        369, 513,
        369, 577,
        304,613,
        297,599,
        356, 568,
        356, 512


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
    float flipperSpeed = 5.0f;
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
    if (IsKeyDown(KEY_DOWN)) {
        // Comprimir spring
        ((b2PrismaticJoint*)spring->joint)->SetMotorSpeed(5.0f);  
       
    }
    else {
        // Soltar spring y lanzar bola
        ((b2PrismaticJoint*)spring->joint)->SetMotorSpeed(-50.0f);
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
        float maxSpeed = 10.0f; // m/s
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
        App->physics->DestroyBody(ball);
        ball = App->physics->CreateCircle(400, 200, 10);
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

    if (debug)
        App->physics->DrawDebug(App->renderer);

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

    if (bodyA == ball && bodyB == resetZone || bodyB == ball && bodyA == resetZone)
    {
        // Reiniciar posición y velocidad de la bola
        resetBall = true;
    }
}