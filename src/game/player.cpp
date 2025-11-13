
#include "player.h"

#include "graphics/mesh.h"
#include "graphics/texture.h"
#include "graphics/shader.h"

#include "framework/camera.h"
#include "framework/input.h"

#include "game/world.h"
#include "game/game.h"
#include "framework/ui.h"
#include "framework/entities/ball.h"

#define RENDER_DEBUG

Player::Player(Mesh* mesh, const Material& material, const std::string& name)
	: EntityMesh(mesh, material, name), current_ball_speed(10.0f)
{
	walk_speed = 1.0f;


	//sword = new EntityMesh(Mesh::Get("data/meshes/sword.obj"), {});
	//addChild(sword);
}

void Player::render(Camera* camera) {
	/*if (camera->type == CAMERA_FIRST_PERSON) {
		return; // Skip rendering the player model
	}
	EntityMesh::render(camera);*/

	/*
#ifdef RENDER_DEBUG
	float sphere_radius = 0.15f;
	float sphere_ground_radius = 0.05f;

	Shader* shader = Shader::Get("data/shaders/basic.vs", "data/shaders/flat.fs");
	Mesh* mesh = Mesh::Get("data/meshes/sphere.obj");
	Matrix44 m = model;
	
	shader->enable();
	
	{
		m.translate(0.0f, sphere_ground_radius, 0.0f);
		m.scale(sphere_ground_radius, sphere_ground_radius, sphere_ground_radius);

		shader->setUniform("u_color", Vector4(1.0f, 0.0f, 0.0f, 1.0f));
		shader->setUniform("u_viewprojection", camera->viewprojection_matrix);
		shader->setUniform("u_model", m);

		mesh->render(GL_LINES);
	}

	{
		m = model;
		m.translate(0.0f, sphere_radius, 0.0f);
		m.scale(sphere_radius, sphere_radius, sphere_radius);

		shader->setUniform("u_color", Vector4(0.0f, 1.0f, 0.0f, 1.0f));
		shader->setUniform("u_model", m);

		mesh->render(GL_LINES);
	}

	shader->disable();
#endif*/
}

void Player::update(float seconds_elapsed) {
	static bool is_walking_sound_playing = false;
	static bool is_running_sound_playing = false;
	static bool is_jumping_sound_playing = false;

	float camera_yaw = World::get_instance()->camera_yaw;

	Matrix44 wYaw;
	wYaw.setRotation(camera_yaw, Vector3(0, 1, 0));

	Vector3 position = model.getTranslation();
	Vector3 teleport_position = World::get_instance()->get_last_ball_collision();

	Vector3 front = wYaw.frontVector();
	Vector3 right = -wYaw.rightVector();
	Vector3 move_dir;

	bool is_moving = false;

	if (Input::isKeyPressed(SDL_SCANCODE_W)) {
		move_dir += front;
		is_moving = true;
	}
	if (Input::isKeyPressed(SDL_SCANCODE_S)) {
		move_dir -= front;
		is_moving = true;
	}
	if (Input::isKeyPressed(SDL_SCANCODE_A)) {
		move_dir -= right;
		is_moving = true;
	}
	if (Input::isKeyPressed(SDL_SCANCODE_D)) {
		move_dir += right;
		is_moving = true;
	}
	if (is_moving && !Input::isKeyPressed(SDL_SCANCODE_SPACE)) {
		if (Input::isKeyPressed(SDL_SCANCODE_LSHIFT)) {
			if (is_walking_sound_playing) {
				Audio::Stop(background_music_channel);
				is_walking_sound_playing = false;
			}
			if (!is_running_sound_playing) {
				background_music_channel = Game::instance->audio.Play("data/audio/running.mp3", 0.5f, BASS_SAMPLE_LOOP);
				is_running_sound_playing = true;
			}
		}
		else {
			if (is_running_sound_playing) {
				Audio::Stop(background_music_channel);
				is_running_sound_playing = false;
			}
			if (!is_walking_sound_playing) {
				background_music_channel = Game::instance->audio.Play("data/audio/walk.mp3", 0.25f, BASS_SAMPLE_LOOP);
				is_walking_sound_playing = true;
			}
		}
	}
	else {
		if (is_walking_sound_playing) {
			Audio::Stop(background_music_channel);
			is_walking_sound_playing = false;
		}
		if (is_running_sound_playing) {
			Audio::Stop(background_music_channel);
			is_running_sound_playing = false;
		}
	}

	float speed_mult = walk_speed;
	if (Input::isKeyPressed(SDL_SCANCODE_LSHIFT)) {
		speed_mult *= 2;
	}

	move_dir.normalize();
	move_dir *= speed_mult;

	velocity += move_dir;

	test_collisions(position, seconds_elapsed);

	position += velocity * seconds_elapsed;

	velocity.x *= 0.25f;
	velocity.z *= 0.25f;

	model.setTranslation(position);

	if (velocity.length() > 0.0f) {
		if (last_camera_yaw != camera_yaw) {
			last_camera_yaw = lerp(last_camera_yaw, camera_yaw, seconds_elapsed * 20.0f);
			model.rotate(last_camera_yaw, Vector3(0, 1, 0));
		}
		else {
			model.rotate(camera_yaw, Vector3(0, 1, 0));
		}
	}
	else {
		model.rotate(last_camera_yaw, Vector3(0, 1, 0));
	}
	// Apply simple head bobbing when moving
	if (velocity.length() > 0.1f) {
		bobbing_timer += seconds_elapsed * bobbing_speed;
		bobbing_offset = sin(bobbing_timer) * bobbing_amount;
	}
	else {
		bobbing_offset = 0.0f; // Reset when stopped
	}

	if (Input::wasKeyPressed(SDL_SCANCODE_G)) {
		if (is_running_sound_playing) {
			Audio::Stop(background_music_channel);
			is_running_sound_playing = false;
		}
		if (is_walking_sound_playing) {
			Audio::Stop(background_music_channel);
			is_walking_sound_playing = false;
		}

		background_music_channel = Game::instance->audio.Play("data/audio/reset.mp3", 0.05f, true);
		recall_position = model.getTranslation();
		recall_timer = recall_time_limit;
		recall_active = true;
	}

	// Recall Mechanic
	if (Input::wasKeyPressed(SDL_SCANCODE_R) && recall_active && recall_timer > 0.0f) {
		if (is_running_sound_playing) {
			Audio::Stop(background_music_channel);
			is_running_sound_playing = false;
		}
		if (is_walking_sound_playing) {
			Audio::Stop(background_music_channel);
			is_walking_sound_playing = false;
		}

		background_music_channel = Game::instance->audio.Play("data/audio/return.mp3", 0.1f, true);
		teleport(recall_position);
		recall_active = false;
	}

	if (recall_active) {
		recall_timer -= seconds_elapsed;
		if (recall_timer <= 0.0f) {
			recall_active = false;
		}
	}
	const float DEFAULT_SPEED = 10.0f;
	const float SLOW_SPEED = 5.0f;
	const float FAST_SPEED = 15.0f;

	if (Input::isKeyPressed(SDL_SCANCODE_2)) {
		current_ball_speed = SLOW_SPEED;
	}
	if (Input::isKeyPressed(SDL_SCANCODE_3)) {
		current_ball_speed = FAST_SPEED;
	}
	if (Input::isKeyPressed(SDL_SCANCODE_4)) {
		current_ball_speed = DEFAULT_SPEED;
	}


	static bool ball_thrown = false; // Prevent multiple throws per key press

	if (Input::isKeyPressed(SDL_SCANCODE_F)) {
		if (!ball_thrown) { // Ensure only one ball is thrown per press
			if (is_running_sound_playing) {
				Audio::Stop(background_music_channel);
				is_running_sound_playing = false;
			}
			if (is_walking_sound_playing) {
				Audio::Stop(background_music_channel);
				is_walking_sound_playing = false;
			}

			background_music_channel = Game::instance->audio.Play("data/audio/throw.mp3", 0.1f, true);
			Vector3 position = model.getTranslation();

			float yaw = World::get_instance()->camera_yaw;
			float pitch = World::get_instance()->camera_pitch;

			// Clamp pitch to prevent extreme values
			pitch = clamp(pitch, -(float)M_PI * 0.4f, (float)M_PI * 0.4f);

			// Start with a forward-facing vector
			Vector3 forward = Vector3(0, 0, 1);

			Matrix44 yawMatrix;
			yawMatrix.setRotation(yaw, Vector3(0, 1, 0));
			forward = yawMatrix * forward;

			// Apply pitch rotation (vertical aiming)
			Matrix44 pitchMatrix;
			pitchMatrix.setRotation(-pitch, yawMatrix.rightVector()); 
			forward = pitchMatrix * forward;

			forward.normalize(); 

			// Adjust the initial spawn position slightly in front of the player
			Vector3 initial_position = position + forward * 0.5f + Vector3(0, 0.5f, 0);

			Vector3 initial_velocity = forward * current_ball_speed;

			// Create and add the ball to the world
			Material ball_material;
			ball_material.diffuse = Texture::Get("data/meshes/colormap1.png");
			ball_material.shader = Shader::Get("data/shaders/basic.vs", "data/shaders/texture.fs");

			Ball* ball = new Ball(Mesh::Get("data/meshes/canon.obj"), ball_material, initial_position, initial_velocity);
			World::get_instance()->addEntity(ball);

			ball_thrown = true;
		}
	}
	else {
		ball_thrown = false; 
	}
	if (teleport_position.length() > 0.001f) { // Ensure teleport position is valid
		model.setTranslation(teleport_position + Vector3(0, 0.1f, 0)); // Slight Y offset

		World::get_instance()->set_last_ball_collision(Vector3(0, 0, 0));
	}
	if (Input::isKeyPressed(SDL_SCANCODE_M)) {
		model.setTranslation(Vector3(-5.0f, 0.5f, -3.0f));
	}

	EntityMesh::update(seconds_elapsed);
}

void Player::test_collisions(Vector3& position, float seconds_elapsed) {

	std::vector<sCollisionData> collisions;
	std::vector<sCollisionData> ground_collisions;

	World::get_instance()->test_scene_collisions(position + velocity * seconds_elapsed, collisions, ground_collisions);

	for (const sCollisionData& collision : collisions) {
		float up_factor = fabsf(collision.col_normal.dot(Vector3::UP));
		if (up_factor > 0.8f) {
			continue;
		}

		Vector3 newDir = velocity.dot(collision.col_normal) * collision.col_normal;
		velocity.x -= newDir.x;
		velocity.z -= newDir.z;

	}

	bool is_grounded = false;

	for (const sCollisionData& collision : ground_collisions) {
		float up_factor = fabsf(collision.col_normal.dot(Vector3::UP));
		if (up_factor > 0.8f) {
			is_grounded = true;
		}
		// if (collision.distance > 1.3f) is_grounded = false;
	}
	if (!is_grounded) {
		if (coyote_timer > 0.0f) {
			coyote_timer -= seconds_elapsed; // Count down
		}
		velocity.y -= 9.8f * seconds_elapsed;
	}
	else {
		coyote_timer = 0.1f; // Reset when grounded
	}

	// **Allow Jumping if Grounded or Within Coyote Time**
	if (Input::wasKeyPressed(SDL_SCANCODE_SPACE) && (is_grounded || coyote_timer > 0.0f)) {
		velocity.y = 6.0f;
		coyote_timer = 0.0f; // Reset after jumping
	}
	else if (is_grounded) {
		velocity.y = 0.0f;
	}
}
void Player::teleport(const Vector3& target_position) {
	model.setTranslation(target_position);
}