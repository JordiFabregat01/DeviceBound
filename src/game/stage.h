#pragma once

#include "framework/input.h"
#include "framework/entities/entity_mesh.h"
#include "framework/audio.h"
#include "graphics/render_to_texture.h"

class Entity;
class Camera;

enum : uint8_t {
	STAGE_MENU,
	STAGE_PLAY,
	STAGE_OPTIONS,
	STAGE_END
};

class Stage {

public:


	virtual void init() {};
	virtual void render(Camera* camera) {};
	virtual void update(double seconds_elapsed) {};


	virtual void onEnter(Stage* prev_stage) {};
	virtual void onLeave(Stage* next_stage) {};

	virtual void onKeyDown(SDL_KeyboardEvent event) {};
	virtual void onKeyUp(SDL_KeyboardEvent event) {};
	virtual void onMouseButtonDown(SDL_MouseButtonEvent event) {};
	virtual void onMouseButtonUp(SDL_MouseButtonEvent event) {};
	virtual void onMouseWheel(SDL_MouseWheelEvent event) {};
	virtual void onMouseMove(SDL_MouseMotionEvent event) {};
	virtual void onGamepadButtonDown(SDL_JoyButtonEvent event) {};
	virtual void onGamepadButtonUp(SDL_JoyButtonEvent event) {};
	virtual void onResize(int width, int height) {};
};

class MenuStage : public Stage {
	EntityUI* background = nullptr;
	EntityUI* play_button = nullptr;
	EntityUI* exit_button = nullptr;
	EntityUI* options_button = nullptr;
	

public:
	HCHANNEL background_music_channel;
	static bool is_music_playing;
	MenuStage();

	void init() override;
	void render(Camera* camera) override;
	void update(double seconds_elapsed) override;
	void onEnter(Stage* prev_stage) override;
	void onLeave(Stage* next_stage) override;

	void onKeyDown(SDL_KeyboardEvent event) override;

};

class OptionsStage : public Stage {
	EntityUI* background = nullptr;
	EntityUI* back_button = nullptr;
	EntityUI* increase_sensitivity_button = nullptr; // Button to increase sensitivity
	EntityUI* decrease_sensitivity_button = nullptr; // Button to decrease sensitivity
	HCHANNEL background_music_channel;
public:

	

	OptionsStage();

	void init() override;
	void render(Camera* camera) override;
	void update(double seconds_elapsed) override;
	void onEnter(Stage* prev_stage) override;
	void onLeave(Stage* next_stage) override;

	void onKeyDown(SDL_KeyboardEvent event) override;

};

class RenderToTexture;

class PlayStage : public Stage {
	HCHANNEL background_music_channel;
	double music_timer;
	double play_timer;
	int current_song_index;
	std::vector<std::string> song_filenames;
	bool help = false;
	Vector3 goal_position = Vector3(140.6f, 13.8f, -0.5f); 
	float goal_radius = 3.0f; 
	Vector3 start_position = Vector3(-5.0f, 0.5f, -3.0f); // Set your actual spawn point



	
public:
	RenderToTexture* pixelate = nullptr;

	PlayStage();

	void init() override;
	void render(Camera* camera) override;
	void update(double seconds_elapsed) override;

	void onEnter(Stage* prev_stage) override;
	void onLeave(Stage* next_stage) override;

	void onKeyDown(SDL_KeyboardEvent event) override;
	void onResize(int width, int height) override;
	void renderMinimap();
	void checkGameEnd(Vector3 player_position);
};


class EndStage : public Stage {
	EntityUI* background = nullptr;
	EntityUI* back_button = nullptr;
	EntityUI* exit_button = nullptr;
	EntityUI* goagain_button = nullptr;

	HCHANNEL background_music_channel;
public:



	EndStage();

	void init() override;
	void render(Camera* camera) override;
	void update(double seconds_elapsed) override;
	void onEnter(Stage* prev_stage) override;
	void onLeave(Stage* next_stage) override;

	void onKeyDown(SDL_KeyboardEvent event) override;

};