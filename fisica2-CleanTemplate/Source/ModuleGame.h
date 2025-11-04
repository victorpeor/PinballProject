#pragma once
#include "Globals.h"
#include "Module.h"
#include "p2Point.h"
#include "raylib.h"

class PhysBody;

class ModuleGame : public Module
{
public:
    ModuleGame(Application* app, bool start_enabled = true);
    ~ModuleGame();

    bool Start() override;
    update_status Update() override;
    update_status PostUpdate() override;
    bool CleanUp() override;

    void OnCollision(PhysBody* bodyA, PhysBody* bodyB);

private:
    // Referencias a los objetos físicos del pinball
    PhysBody* ball = nullptr;
    PhysBody* leftFlipper = nullptr;
    PhysBody* rightFlipper = nullptr;
    PhysBody* spring = nullptr;
    PhysBody* resetZone = nullptr;
    PhysBody* bumper = nullptr;
    PhysBody* bumper2 = nullptr;
    PhysBody* bumper3 = nullptr;
    PhysBody* Tbumper = nullptr;
    PhysBody* Tbumper2 = nullptr;
    PhysBody* collectible1 = nullptr;
    PhysBody* collectible2 = nullptr;
    PhysBody* collectible3 = nullptr;
    // Texturas opcionales para renderizar
    Texture2D texBall{};
    Texture2D texSpring{};
    Texture2D texMap{};
    Texture2D texFlipperLeft{};
    Texture2D texFlipperRight{};
    Texture2D textCollectible{};

    // Efectos de sonido, puntuación, etc.
    uint bonus_fx = 0;
    bool debug = false;
    bool resetBall = false;
    // Función auxiliar para renderizar objetos
    //void DrawBall();
};