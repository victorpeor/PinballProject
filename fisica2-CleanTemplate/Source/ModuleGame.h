#pragma once

#include "Globals.h"
#include "Module.h"

#include "p2Point.h"

#include "raylib.h"
#include <vector>

class PhysBody;
class PhysicEntity;

class ModuleGame : public Module
{
public:
	ModuleGame(Application* app, bool start_enabled = true);
	~ModuleGame();

	bool Start() override;
	update_status Update() override;
	bool CleanUp() override;

private:
	// === Texturas de los sprites ===
	Texture2D texMap = { 0 };             // Fondo / mapa
	Texture2D texBall = { 0 };            // Bola
	Texture2D texFlipperL = { 0 };        // Palanca izquierda
	Texture2D texFlipperR = { 0 };        // Palanca derecha (inverted)

	// === Posiciones (en píxeles, coordenadas de pantalla) ===
	// Ajusta estos valores para colocarlos bien sobre tu mapa
	Vector2 posMap = { 0.0f, 0.0f };
	Vector2 posBall = { 300.0f, 200.0f };
	Vector2 posFlipperL = { 240.0f, 520.0f };
	Vector2 posFlipperR = { 360.0f, 520.0f };

	// === Escala de dibujo (1.0 = tamaño original) y rotación (en grados) ===
	float mapScale = 1.0f;
	float ballScale = 1.0f;
	float flipperScale = 1.0f;
	float flipperLRotation = 0.0f;
	float flipperRRotation = 0.0f;

	// Helper: dibuja una textura centrada opcionalmente (útil para futuras rotaciones)
	void DrawTextureAt(const Texture2D& tex, Vector2 pos, float scale = 1.0f, float rotationDegrees = 0.0f, bool centered = false) const;

	// Helper: comprueba si una textura está cargada
	inline bool IsLoaded(const Texture2D& tex) const { return tex.id != 0; }
};
