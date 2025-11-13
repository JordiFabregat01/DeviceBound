#include "game.h"
#include "framework/utils.h"
#include "graphics/mesh.h"
#include "graphics/texture.h"
#include "graphics/fbo.h"
#include "graphics/shader.h"
#include "framework/input.h"
#include "scene_parser.h"
#include <cmath>
#include "framework/entities/entity.h"
#include "framework/entities/entity_mesh.h"
#include "framework/entities/ball.h"
#include "framework/ui.h"
#include "stage.h"
#include "world.h"
#include "player.h"
#include "framework/audio.h"


//some globals

Game* Game::instance = nullptr;


Game::Game(int window_width, int window_height, SDL_Window* window)
{
	this->window_width = window_width;
	this->window_height = window_height;
	this->window = window;
	instance = this;
	must_exit = false;

	fps = 0;
	frame = 0;
	time = 0.0f;
	elapsed_time = 0.0f;
	mouse_locked = false;

	Audio::Init();

	stages[STAGE_MENU] = new MenuStage();
	stages[STAGE_OPTIONS] = new OptionsStage();
	stages[STAGE_PLAY] = new PlayStage();
	stages[STAGE_END] = new EndStage();
	

	for (auto& entry : stages)
	{
		int id = entry.first;
		Stage* stage = entry.second;
		if (stage)
			stage->init();
	}

	current_stage = stages[STAGE_MENU];
	current_stage->onEnter(nullptr);

	goToStage(STAGE_MENU);


	// OpenGL flags
	glEnable(GL_CULL_FACE); //render both sides of every triangle
	glEnable(GL_DEPTH_TEST); //check the occlusions using the Z buffer

	audio.Init();
	audio.Get("data/audio/menumusic.mp3");
	audio.Get("data/audio/menumusic2.mp3");
	audio.Get("data/audio/menumusic3.mp3");


	// Create our camera
	camera = new Camera();
	camera->lookAt(Vector3(0.f, 1.f, 10.f), Vector3(0.f, 0.f, 0.f), Vector3(0.f, 1.f, 0.f)); //position the camera and point to 0,0,0
	camera->setPerspective(70.f, window_width / (float)window_height, 0.01f, 1000.f); //set the projection, we want to be perspective

	// Hide the cursor
	SDL_ShowCursor(!mouse_locked); //hide or show the mouse

}

//what to do when the image has to be draw
void Game::render(void)
{
	// Set the clear color (the background color)
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (current_stage) {
		current_stage->render(camera);
	}

	renderHUD();

	// Swap between front buffer and back buffer
	SDL_GL_SwapWindow(this->window);
}

void Game::update(double seconds_elapsed) {

	if (current_stage) {
		current_stage->update(seconds_elapsed);
	}
}


void Game::goToStage(uint8_t stage_id)
{
	Stage* new_stage = stages[stage_id];
	assert(new_stage);

	if (current_stage) {
		current_stage->onLeave(new_stage);
	}

	new_stage->onEnter(current_stage);

	current_stage = new_stage;

}

//Keyboard event handler (sync input)
void Game::onKeyDown(SDL_KeyboardEvent event)
{
	switch (event.keysym.sym)
	{
	//case SDLK_ESCAPE: must_exit = true; break; //ESC key, kill the app
	case SDLK_F1: Shader::ReloadAll(); break;
	}
	if (current_stage) {
		current_stage->onKeyDown(event);
	}
}

void Game::onKeyUp(SDL_KeyboardEvent event)
{
	if (current_stage) {
		current_stage->onKeyDown(event);
	}
}

void Game::onMouseButtonDown(SDL_MouseButtonEvent event)
{
	if (event.button == SDL_BUTTON_MIDDLE) //middle mouse
	{
		mouse_locked = !mouse_locked;
		SDL_ShowCursor(!mouse_locked);
		SDL_SetRelativeMouseMode((SDL_bool)(mouse_locked));
	}
}

void Game::onMouseButtonUp(SDL_MouseButtonEvent event)
{

}

void Game::onMouseWheel(SDL_MouseWheelEvent event)
{
}

void Game::onGamepadButtonDown(SDL_JoyButtonEvent event)
{

}

void Game::onGamepadButtonUp(SDL_JoyButtonEvent event)
{

}

void Game::onResize(int width, int height)
{
	glViewport(0, 0, width, height);
	window_width = width;
	window_height = height;
	if (current_stage) {
		current_stage->onResize(width, height);
	}
}
void Game::setMouseLocked(bool must_lock)
{
	SDL_ShowCursor(!must_lock);
	SDL_SetRelativeMouseMode((SDL_bool)must_lock);
	mouse_locked = must_lock;
}
void Game::renderHUD()
{
	glDisable(GL_DEPTH_TEST); // Ensure UI renders on top
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	Player* player = World::get_instance()->player;
	if (player && player->isRecallActive()) {
		float progress = player->getRecallTimer() / player->getRecallTimeLimit();
		UI::renderProgressBar(Vector2(120, 100), Vector2(200, 20), progress, Vector4(1.0f, 1.0f, 0.0f, 1.0f));
	}

	glEnable(GL_DEPTH_TEST); // Re-enable depth test for 3D rendering
}
