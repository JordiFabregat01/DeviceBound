

#pragma once

#include "framework/entities/entity.h"
#include "framework/entities/entity_mesh.h"
#include "graphics/material.h"
#include "framework/audio.h"

class Player : public EntityMesh {

	float walk_speed = 0.0f;

	Vector3 velocity = Vector3(0.0f);

	EntityMesh* sword = nullptr;

	float last_camera_yaw = 0.0f;

	

	Vector3 recall_position;
	bool recall_active = false;
	float recall_timer = 0.0f;
	static constexpr float recall_time_limit = 15.0f; // 15 seconds
	bool recall_pending = false;

	float coyote_time = 0.1f;   // Allow jumping 0.1s after leaving ground
	float coyote_timer = 0.0f;  // Tracks time since last on the ground
	bool is_grounded = false;   // Stores current grounded state
	bool was_grounded = false;  // Stores previous frame's ground state
	bool running = false;
	bool walking = false;

	



public:
	HCHANNEL background_music_channel;
	float current_ball_speed;
	float player_height = 0.5f;   // Height of player
	float bobbing_offset = 0.0f;  // Stores height change
	float bobbing_timer = 0.0f;   // Timer for bobbing
	bool bobbing_up = true;       // Toggles direction
	float bobbing_speed = 6.0f;   // Speed of bobbing
	float bobbing_amount = 0.05f; // How much the head moves

	Player() {};
	Player(Mesh* mesh, const Material& material, const std::string& name = "");

	void update(float seconds_elapsed) override;
	void render(Camera* camera) override;

	void test_collisions(Vector3& position, float seconds_elapsed);
	void teleport(const Vector3& target_position);
	float getRecallTimer() const { return recall_timer; }
	float getRecallTimeLimit() const { return recall_time_limit; }
	bool isRecallActive() const { return recall_active; }
};