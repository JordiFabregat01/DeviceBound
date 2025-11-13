#include "stage.h"
#include "framework/utils.h"
#include "framework/camera.h"
#include "framework/entities/entity.h"
#include "framework/entities/entity_mesh.h"
#include "framework/input.h"
#include "framework/ui.h"

#include "graphics/mesh.h"
#include "graphics/texture.h"
#include "graphics/shader.h"

#include "game/scene_parser.h"
#include "game/world.h"
#include "game/game.h"
#include "game/player.h"

#include "framework/audio.h"

#include <iomanip> // For std::setprecision
#include <sstream> // For std::ostringstream

bool MenuStage::is_music_playing = false; // Define the static member


MenuStage::MenuStage()
{
}

void MenuStage::init() {

	int width = Game::instance->window_width;
	int height = Game::instance->window_height;

	Material background_mat;
	background_mat.diffuse = Texture::Get("data/ui/plss.png");
	background = new EntityUI(Vector2(width*0.5, height*0.5), Vector2(width, height), background_mat);

	{
		Material play_mat;
		play_mat.diffuse = Texture::Get("data/ui/startings.png");
		play_button = new EntityUI(Vector2(width * 0.5, height * 0.65), Vector2(100, 40), play_mat, UI_BUTTON_PLAY);
		background->addChild(play_button);
	}
	{
		Material exit_mat;
		exit_mat.diffuse = Texture::Get("data/ui/menuquit.png");
		exit_button = new EntityUI(Vector2(width * 0.5, height * 0.9), Vector2(110, 40), exit_mat, UI_BUTTON_EXIT);
		background->addChild(exit_button);
	}
	{
		Material options_mat;
		options_mat.diffuse = Texture::Get("data/ui/options.png");
		options_button = new EntityUI(Vector2(width * 0.5, height * 0.7), Vector2(110, 40), options_mat, UI_BUTTON_OPTIONS);
		background->addChild(options_button);
	}
}

void MenuStage::onEnter(Stage* prev_stage) {
	Game::instance->setMouseLocked(false);
	background_music_channel = Game::instance->audio.Play("data/audio/menumusic.mp3", 0.01f, BASS_SAMPLE_LOOP);
}
void MenuStage::onLeave(Stage* next_stage) {
	Audio::Stop(background_music_channel);

}


void MenuStage::render(Camera* camera) {

	background->render(World::get_instance()->camera2D);

}

void MenuStage::update(double seconds_elapsed) {

	if (background) {
		background->update(seconds_elapsed);
	}
}

void MenuStage::onKeyDown(SDL_KeyboardEvent event) {
}

OptionsStage::OptionsStage()
{
}
void OptionsStage::init() {
	int width = Game::instance->window_width;
	int height = Game::instance->window_height;

	Material background_mat;
	background_mat.diffuse = Texture::Get("data/ui/plss.png");
	background = new EntityUI(Vector2(width * 0.5, height * 0.5), Vector2(width, height), background_mat);
	{
		Material back_mat;
		back_mat.diffuse = Texture::Get("data/ui/menuquit.png");
		back_button = new EntityUI(Vector2(width * 0.5, height * 0.9), Vector2(110, 40), back_mat, UI_BUTTON_BACK);
		background->addChild(back_button);
	}
	{
		Material increase_mat;
		increase_mat.diffuse = Texture::Get("data/ui/increase.png");
		increase_sensitivity_button = new EntityUI(Vector2(width * 0.45, height * 0.7), Vector2(100, 100), increase_mat, UI_BUTTON_INCREASE_SENSITIVITY);
		background->addChild(increase_sensitivity_button);
	}
	{
		Material decrease_mat;
		decrease_mat.diffuse = Texture::Get("data/ui/decrease.png");
		decrease_sensitivity_button = new EntityUI(Vector2(width * 0.55, height * 0.7), Vector2(100, 100), decrease_mat, UI_BUTTON_DECREASE_SENSITIVITY);
		background->addChild(decrease_sensitivity_button);
	}
}

void OptionsStage::onEnter(Stage* prev_stage) {
	Game::instance->setMouseLocked(false);
	background_music_channel = Game::instance->audio.Play("data/audio/menumusic2.mp3", 0.01f, BASS_SAMPLE_LOOP);
	
}
void OptionsStage::onLeave(Stage* next_stage) {
	Audio::Audio::Stop(background_music_channel);
}

void OptionsStage::render(Camera* camera) {
	int width = Game::instance->window_width;
	int height = Game::instance->window_height;

	background->render(World::get_instance()->camera2D);

	float sensitivity = World::get_instance()->mouse_speed;
	float rounded_sensitivity = std::round(sensitivity * 100.0f) / 100.0f;

	std::ostringstream sensitivity_stream;
	sensitivity_stream << std::fixed << std::setprecision(2) << rounded_sensitivity;
	std::string sensitivity_text = "Sensitivity: " + sensitivity_stream.str();


	drawText(width * 0.4, height * 0.6, sensitivity_text, Vector3(0, 0, 0), 5.0f);
}

void OptionsStage::update(double seconds_elapsed) {



	if (background) {
		background->update(seconds_elapsed);
	}
}

void OptionsStage::onKeyDown(SDL_KeyboardEvent event) {
}

PlayStage::PlayStage()
{
	pixelate = nullptr;
	music_timer = 0.0;
	current_song_index = 0;
	song_filenames = {
	"data/audio/menumusic3.mp3",
	"data/audio/menumusic5.mp3",
	"data/audio/menumusic6.mp3"
	};
}

void PlayStage::init()
{
	pixelate = new RenderToTexture();
	if (!pixelate->create(800, 600, true)) {
		delete pixelate;
		pixelate = nullptr;
	}
	music_timer = 0.0;
	play_timer = 0.0;
}



void PlayStage::onEnter(Stage* prev_stage)
{
	World::get_instance()->camera_type = CAMERA_FIRST_PERSON;
	World::get_instance()->free_camera = false; // Ensure free camera is off
	Game::instance->setMouseLocked(true);
	background_music_channel = Game::instance->audio.Play(song_filenames[current_song_index], 0.01f, BASS_SAMPLE_LOOP);
	music_timer = 0.0;
}
void PlayStage::onLeave(Stage* next_stage) {
	Audio::Stop(background_music_channel);
	Audio::Stop(World::get_instance()->player->background_music_channel);
}

void PlayStage::render(Camera* camera)
{
	int width = Game::instance->window_width;
	int height = Game::instance->window_height;
	World* world = World::get_instance();
	world->render();
	/*if (pixelate) {
		pixelate->enable();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		// Render scene
		World* world = World::get_instance();
		world->render();
		pixelate->disable();
	}
	else {
		std::cerr << "RenderToTexture object is not initialized." << std::endl;
	}*/

	glDisable(GL_DEPTH_TEST);

	//pixelate->toViewport(Shader::Get("data/shaders/screen.vs", "data/shaders/piexlate.fs"));

	renderMinimap();

	int minutes = static_cast<int>(play_timer) / 60; // Use play_timer here
	int seconds = static_cast<int>(play_timer) % 60; // Use play_timer here
	std::string timer_text = "Time: " + std::to_string(minutes) + ":" + (seconds < 10 ? "0" : "") + std::to_string(seconds);
	drawText(width * 0.05, height * 0.05, timer_text, Vector3(1, 1, 1), 2.0f);

	// Render the crosshair
	int centerX = width / 2;
	int centerY = height / 2;
	int crosshairSize = 10; // Size of the crosshair

	// Draw horizontal line
	drawText(centerX + crosshairSize - 2.f, centerY, ".", Vector3(1, 1, 1), 2.0f);

	if (help == true) {
		drawText(width * 0.05, height * 0.55, "Press Tab to go to options", Vector3(1, 1, 1), 2.0f);
		drawText(width * 0.05, height * 0.6, "Press G to save your current position for 15 seconds", Vector3(1, 1, 1), 2.0f);
		drawText(width * 0.05, height * 0.65, "Press R to return to that position", Vector3(1, 1, 1), 2.0f);
		drawText(width * 0.05, height * 0.7, "Press F to throw the teleportation device", Vector3(1, 1, 1), 2.0f);
		drawText(width * 0.05, height * 0.75, "Press M to restart", Vector3(1, 1, 1), 2.0f);
		drawText(width * 0.05, height * 0.8, "Press 2 for slow ball throw", Vector3(1, 1, 1), 2.0f);
		drawText(width * 0.05, height * 0.85, "Press 3 for fast ball throw", Vector3(1, 1, 1), 2.0f);
		drawText(width * 0.05, height * 0.9, "Press 4 for normal ball throw", Vector3(1, 1, 1), 2.0f);
	}
	else {
		drawText(width * 0.05, height * 0.9, "Press H to see controls", Vector3(1, 1, 1), 2.0f);
	}

	glEnable(GL_DEPTH_TEST);

}
void PlayStage::renderMinimap() {

	int width = Game::instance->window_width;
	int height = Game::instance->window_height;

	glClear(GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	{
		int size = 200;
		int margin = 10;
		glViewport(width - size - margin, height - size - margin, size, size);
	}

	World* world = World::get_instance();
	EntityMesh* current_player = world->player;

	current_player->rendering_minimap = true;

	Camera cam;
	cam.setPerspective(60.f, 1.f, 0.1, 100.0f);

	Vector3 center = world->player->model.getTranslation();
	Vector3 eye = center + Vector3(0.0f, 10.0f, 0.0f);
	Vector3 up = Vector3(0.0f, 0.0f, 1.0f);

	Matrix44 mYaw;
	mYaw.setRotation(world->camera_yaw, Vector3(0.0f, 1.0f, 0.0f));

	up = mYaw.rotateVector(up);

	cam.lookAt(
		eye,
		center,
		up
	);
	cam.enable();

	World::get_instance()->root->render(&cam);

	glViewport(0, 0, width, height);

	drawText(width * 0.94, height * 0.085 , "^", Vector3(1, 0, 0), 2.0f);
	drawText(width * 0.94, height * 0.095, "o", Vector3(1, 0, 0), 2.0f);

}
void PlayStage::update(double seconds_elapsed)
{
	World* world = World::get_instance();
	world->update(seconds_elapsed);

	// Update the music timer
	music_timer += seconds_elapsed;
	play_timer += seconds_elapsed;
	if (music_timer >= 70.0) { // Assuming each song lasts approximately 1 minute
		Audio::Stop(background_music_channel);

		// Move to the next song in the list
		current_song_index = (current_song_index + 1) % song_filenames.size();

		// Play the next song
		background_music_channel = Game::instance->audio.Play(song_filenames[current_song_index], 0.01f, BASS_SAMPLE_LOOP);
		music_timer = 0.0; // Reset the timer
	}
	if (Input::isKeyPressed(SDL_SCANCODE_TAB)) {
		Game::instance->goToStage(STAGE_OPTIONS);
	}
	if (Input::wasKeyPressed(SDL_SCANCODE_H)) {
		if (help == false) {
			help = true;
		}
		else if(help == true){
			help = false;
		}
	}
	Vector3 player_position = world->player->model.getTranslation();
	checkGameEnd(player_position);
}


void PlayStage::onKeyDown(SDL_KeyboardEvent event)
{
	switch (event.keysym.sym)
	{
	case SDLK_TAB:
	{
		bool& free_camera = World::get_instance()->free_camera;
		free_camera = !free_camera;
		Game::instance->setMouseLocked(!free_camera);
		break;
	}
	case SDLK_1:
	{
		World::get_instance()->camera_type = CAMERA_FIRST_PERSON;
		World::get_instance()->free_camera = false; // Ensure free_camera is disabled
		Game::instance->setMouseLocked(true); // Lock the mouse for first-person mode
		break;
	}
	}
}
void PlayStage::onResize(int width, int height)
{
	World* world = World::get_instance();
	world->camera->aspect = width / (float)height;
}
void PlayStage::checkGameEnd(Vector3 player_position) {
	if ((player_position - goal_position).length() <= goal_radius) {
		Game::instance->goToStage(STAGE_END); // Transition to an ending screen
		World::get_instance()->player->model.setTranslation(start_position);
	}
}
EndStage::EndStage() {

}

void EndStage::init() {
	int width = Game::instance->window_width;
	int height = Game::instance->window_height;

	Material background_mat;
	background_mat.diffuse = Texture::Get("data/ui/escape.png");
	background = new EntityUI(Vector2(width * 0.5, height * 0.5), Vector2(width, height), background_mat);

	{
		Material exit_mat;
		exit_mat.diffuse = Texture::Get("data/ui/menuquit.png");
		exit_button = new EntityUI(Vector2(width * 0.5, height * 0.85), Vector2(100, 40), exit_mat, UI_BUTTON_EXIT);
		background->addChild(exit_button);
	}
	{
		Material return_mat;
		return_mat.diffuse = Texture::Get("data/ui/goagain.png");
		goagain_button = new EntityUI(Vector2(width * 0.5, height * 0.9), Vector2(100, 40), return_mat, UI_BUTTON_BACK);
		background->addChild(goagain_button);
	}
}
void EndStage::onEnter(Stage* prev_stage) {
	Game::instance->setMouseLocked(false);
	background_music_channel = Game::instance->audio.Play("data/audio/finalmusic.mp3", 0.1f, BASS_SAMPLE_LOOP);
}

void EndStage::onLeave(Stage* next_stage) {
	Audio::Audio::Stop(background_music_channel);
}

void EndStage::render(Camera* camera) {
	int width = Game::instance->window_width;
	int height = Game::instance->window_height;
	background->render(World::get_instance()->camera2D);
	drawText(width * 0.1, height * 0.9, "Thank you for playing my game\n hope you enjoyed it :)", Vector3(1, 1, 1), 3.0f);
	drawText(width * 0.35, height * 0.4, "YOU ESCAPED!", Vector3(1, 1, 0), 9.0f);


}

void EndStage::update(double seconds_elapsed) {
	if (background) {
		background->update(seconds_elapsed);
	}
}

void EndStage::onKeyDown(SDL_KeyboardEvent event) {
}