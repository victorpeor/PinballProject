#include "Application.h"
#include "ModuleGame.h"
#include "Globals.h"

// raylib ya está incluido en el .h

ModuleGame::ModuleGame(Application* app, bool start_enabled) : Module(app, start_enabled)
{
}

ModuleGame::~ModuleGame()
{
}

bool ModuleGame::Start()
{
	// --- Cargar texturas desde la carpeta Assets (ruta relativa al ejecutable/working directory) ---
	// Cambia las extensiones si tus archivos no son .png
	texMap = LoadTexture("Assets/map.png");
	texBall = LoadTexture("Assets/bola.png");
	texFlipperL = LoadTexture("Assets/palanca.png");
	texFlipperR = LoadTexture("Assets/palanca_inverted.png");

	// Comprobación básica de carga
	if (!IsLoaded(texMap)) { LOG("ERROR: No se pudo cargar Assets/map.png"); return false; }
	if (!IsLoaded(texBall)) { LOG("ERROR: No se pudo cargar Assets/bola.png"); return false; }
	if (!IsLoaded(texFlipperL)) { LOG("ERROR: No se pudo cargar Assets/palanca.png"); return false; }
	if (!IsLoaded(texFlipperR)) { LOG("ERROR: No se pudo cargar Assets/palanca_inverted.png"); return false; }

	// Si necesitas ajustar escalas iniciales:
	// mapScale = 1.0f; ballScale = 1.0f; flipperScale = 1.0f;

	LOG("ModuleGame::Start() - Texturas cargadas y listas para dibujar.");
	return true;
}

update_status ModuleGame::Update()
{
	// NOTA: Asumo que otro módulo (render/window) ya hace BeginDrawing()/EndDrawing().
	// Si no, y este módulo debe hacerlo, descomenta:
	// BeginDrawing();
	// ClearBackground(BLACK);

	// 1) Dibujar MAP primero (fondo)
	DrawTextureAt(texMap, posMap, mapScale, 0.0f, /*centered=*/false);

	// 2) Bola
	DrawTextureAt(texBall, posBall, ballScale, 0.0f, /*centered=*/false);

	// 3) Palancas (puedes cambiar centered=true y rotarlas luego)
	DrawTextureAt(texFlipperL, posFlipperL, flipperScale, flipperLRotation, /*centered=*/false);
	DrawTextureAt(texFlipperR, posFlipperR, flipperScale, flipperRRotation, /*centered=*/false);

	// Si este módulo gestiona el frame:
	// EndDrawing();

	return update_status::UPDATE_CONTINUE;
}

bool ModuleGame::CleanUp()
{
	// Liberar texturas si están cargadas
	if (IsLoaded(texMap)) { UnloadTexture(texMap);      texMap = { 0 }; }
	if (IsLoaded(texBall)) { UnloadTexture(texBall);     texBall = { 0 }; }
	if (IsLoaded(texFlipperL)) { UnloadTexture(texFlipperL); texFlipperL = { 0 }; }
	if (IsLoaded(texFlipperR)) { UnloadTexture(texFlipperR); texFlipperR = { 0 }; }

	LOG("ModuleGame::CleanUp() - Texturas liberadas.");
	return true;
}

void ModuleGame::DrawTextureAt(const Texture2D& tex, Vector2 pos, float scale, float rotationDegrees, bool centered) const
{
	if (!IsLoaded(tex)) return;

	// Dimensiones escaladas
	const float w = tex.width * scale;
	const float h = tex.height * scale;

	if (!centered && rotationDegrees == 0.0f && scale == 1.0f)
	{
		// Camino rápido: dibuja sin transformaciones
		DrawTexture(tex, (int)pos.x, (int)pos.y, WHITE);
		return;
	}

	// Usamos DrawTexturePro para soportar escala, rotación y anclaje
	Rectangle src{ 0, 0, (float)tex.width, (float)tex.height };
	Rectangle dst{ pos.x, pos.y, w, h };
	Vector2 origin{ 0.0f, 0.0f };

	if (centered)
	{
		// Origen al centro del sprite
		origin = { w * 0.5f, h * 0.5f };
		// Cuando el origen está centrado, el dst.x/dst.y deben ser el centro visual
		// (tal como ya pasamos pos como centro)
	}
	// Si NO está centrado, pos es la esquina superior izquierda.

	DrawTexturePro(tex, src, dst, origin, rotationDegrees, WHITE);
}
