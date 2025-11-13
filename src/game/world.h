
#pragma once

#include "framework/utils.h"
#include "framework/entities/entity.h"
#include "framework/entities/entity_mesh.h"

class Camera;
class Player;

enum : uint8 {
	CAMERA_FIRST_PERSON,
	CAMERA_TYPE_COUNT
};

class World {
	

	void update_fp_camera(float seconds_elapsed);

public:
	static World* instance;

	static World* get_instance() {
		if (instance != nullptr)
			return instance;
		instance = new World();
		return instance;
	}

	World();
	~World() { instance = nullptr; }

	Entity* root = nullptr;

	EntityMesh* skybox = nullptr;

	Player* player = nullptr;

	Camera* camera = nullptr;
	Camera* camera2D = nullptr;

	float camera_yaw = 0.f;
	float camera_pitch = 0.f;
	float camera_speed = 2.0f;
	float mouse_speed = 0.25f;

	Vector3 last_ball_collision = Vector3(0, 0, 0);


	bool free_camera = true;
	uint8_t camera_type = CAMERA_FIRST_PERSON;

	void render();
	void update(double seconds_elapsed);

	std::vector<Entity*> entities_to_destroy;

	void addEntity(Entity* entity);
	void destroyEntity(Entity* entity);

	sCollisionData raycast(const Vector3& origin, const Vector3& direction, int layer = eCollisionFilter::ALL, bool closest = true, float max_ray_distance = 1e20f);
	void test_scene_collisions(const Vector3& position, std::vector<sCollisionData>& collisions, std::vector<sCollisionData>& ground_collisions);
	void set_last_ball_collision(const Vector3& position) { last_ball_collision = position; }
	Vector3 get_last_ball_collision() { return last_ball_collision; }

};