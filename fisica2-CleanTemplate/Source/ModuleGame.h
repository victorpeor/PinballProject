#pragma once
#include "Globals.h"
#include "Module.h"
#include "p2Point.h"
#include "raylib.h"

class PhysBody;

enum class GameState { MENU, PLAYING, GAMEOVER };

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
    // --- Estado de juego ---
    GameState currentState = GameState::MENU;
    GameState nextState = GameState::MENU;

    // --- Transiciones (fade) ---
    bool fading = false;
    bool fadeIn = false;
    bool fadeOut = false;
    float fadeAlpha = 0.0f;
    float fadeSpeed = 0.02f; // Ajusta a gusto

    // --- Referencias físicas del pinball ---
    PhysBody* ball = nullptr;
    PhysBody* leftFlipper = nullptr;
    PhysBody* leftFlipper2 = nullptr;
    PhysBody* rightFlipper = nullptr;
    PhysBody* rightFlipper2 = nullptr;
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

    // --- Texturas ---
    Texture2D texBall{};
    Texture2D texSpring{};
    Texture2D texMap{};
    Texture2D texFlipperLeft{};
    Texture2D texFlipperRight{};
    Texture2D textCollectible{};
    Texture2D texLife{};
    Texture2D texLose{};
    Texture2D texMenu{};

    // --- Flags y datos de juego ---
    int lives = 3;
    bool loseLifePending = false;
    int highScore = 0;
    int previousScore = 0;

    // --- Audio ---
    unsigned int springsound = 0;
    unsigned int background_music = 0;
    unsigned int flipersound = 0;
    unsigned int ballvoid = 0;
    unsigned int newball = 0;
    uint bonus_fx = 0;

    // --- Varios ---
    bool debug = false;
    bool resetBall = false;
    bool leftFlipperPressed = false;
    bool rightFlipperPressed = false;
    bool springPressed = false;

    // --- Puntuación básica ---
    const int POINTS_COLLECTIBLE = 100;
    const int POINTS_BUMPER_BIG = 50;
    const int POINTS_BUMPER_SMALL = 25;
    int collectible_left = 3;
    int score = 0;

    // --- Helpers de estado/transición ---
    void StartTransitionTo(GameState target);
    void ApplyStateChangeAtBlack();   // se ejecuta con pantalla a negro (alpha=1)
    void EnterState(GameState s);     // acciones al entrar en s
    void DrawMenu();
    void DrawGame();
    void DrawGameOver();
};
