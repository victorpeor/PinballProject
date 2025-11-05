#include "Globals.h"
#include "Application.h"
#include "ModuleAudio.h"
#include "raylib.h"

#define MAX_FX 20

ModuleAudio::ModuleAudio(Application* app, bool start_enabled) : Module(app, start_enabled)
{
    max_fx = MAX_FX;
    sounds = new Sound[MAX_FX];
    music = nullptr;
    fx_count = 0;
}

ModuleAudio::~ModuleAudio()
{
    delete[] sounds;
}

bool ModuleAudio::Init()
{
    LOG("Initializing Audio System");
    InitAudioDevice();
    return true;
}

update_status ModuleAudio::Update()
{
    if (music != nullptr) {
        UpdateMusicStream(*music);
    }
    return UPDATE_CONTINUE;
}

bool ModuleAudio::CleanUp()
{
    LOG("Freeing audio system");

    // Liberar música
    if (music != nullptr) {
        StopMusicStream(*music);
        UnloadMusicStream(*music);
        delete music;
        music = nullptr;
    }

    // Liberar efectos de sonido
    for (int i = 0; i < fx_count; i++) {
        UnloadSound(sounds[i]);
    }
    fx_count = 0;

    CloseAudioDevice();
    return true;
}

unsigned int ModuleAudio::LoadFx(const char* path)
{
    if (fx_count < max_fx) {
        sounds[fx_count] = LoadSound(path);
        if (sounds[fx_count].frameCount > 0) {
            fx_count++;
            return fx_count; // Retorna el ID (1-based)
        }
    }
    return 0;
}

void ModuleAudio::PlayFx(unsigned int fx_id)
{
    if (fx_id > 0 && fx_id <= fx_count) {
        PlaySound(sounds[fx_id - 1]);
    }
}

void ModuleAudio::PlayMusic(const char* path)
{
    // Detener música anterior
    if (music != nullptr) {
        StopMusicStream(*music);
        UnloadMusicStream(*music);
        delete music;
    }

    // Cargar nueva música
    music = new Music(LoadMusicStream(path));
    if (music->frameCount > 0) {
        PlayMusicStream(*music);
    }
}

void ModuleAudio::StopMusic()
{
    if (music != nullptr) {
        StopMusicStream(*music);
    }
}