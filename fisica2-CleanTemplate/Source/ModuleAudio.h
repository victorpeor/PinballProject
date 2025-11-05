#pragma once
#include "Module.h"

// Forward declaration de tipos de raylib
typedef struct Music Music;
typedef struct Sound Sound;

class ModuleAudio : public Module
{
public:
    ModuleAudio(Application* app, bool start_enabled = true);
    ~ModuleAudio();

    bool Init();
    update_status Update();
    bool CleanUp();

    // Efectos de sonido
    unsigned int LoadFx(const char* path);
    void PlayFx(unsigned int fx_id);

    // Música
    void PlayMusic(const char* path);
    void StopMusic();

private:
    Sound* sounds;
    Music* music;
    int fx_count;
    int max_fx;
};