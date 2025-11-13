
#pragma once

#include "framework/includes.h"
#include "framework/framework.h"
#include "framework/entities/entity.h"
#include "graphics/mesh.h"

#include "graphics/material.h"

class Camera;
class Mesh;
class Material;
class EntityCollider;

enum eCollisionFilter : uint8_t {
	NONE = 0,
	FLOOR = 1 << 0,
	WALL = 1 << 1,
	SCENARIO = FLOOR | WALL,
	ALL = 0xff
};

class EntityMesh : public Entity {

public:
	EntityMesh() {};
	EntityMesh(Mesh* new_mesh, const Material& new_material);
	EntityMesh(Mesh* new_mesh, const Material& new_material, const std::string& name);
	virtual ~EntityMesh() {};

	Mesh* mesh = nullptr;

	Material material;

	bool isInstanced = false;
	float distance_culling = 100.0f;
	std::vector<Matrix44> models;

	bool rendering_minimap;

	void render(Camera* camera) override;
	void update(float delta_time) override;

	void addInstance(const Matrix44& model);
};

class EntityCollider;

struct sCollisionData {
	Vector3 col_point;
	Vector3 col_normal;
	float distance = 3.4e+38F;
	bool collided = false;
	EntityCollider* collider = nullptr;
};

class EntityCollider : public EntityMesh {

	void getCollisionsWithModel(const Matrix44& m, const Vector3& center, std::vector<sCollisionData>& collisions, std::vector<sCollisionData>& ground_collisions);

public:

	int layer = eCollisionFilter::SCENARIO;
	bool is_static = true;

	EntityCollider() {};
	EntityCollider(Mesh* mesh, const Material& material)
		: EntityMesh(mesh, material) {
	};

	void getCollisions(const Vector3& target_position, std::vector<sCollisionData>& collisions, std::vector<sCollisionData>& ground_collisions);
	int getLayer() { return layer; }
};

enum eUIButtonID {
	UI_BUTTON_UNDEFINED,
	UI_BUTTON_PLAY,
	UI_BUTTON_EXIT,
	UI_BUTTON_OPTIONS,
	UI_BUTTON_BACK,
	UI_BUTTON_INCREASE_SENSITIVITY,
	UI_BUTTON_DECREASE_SENSITIVITY,
	UI_BUTTON_FULLSCREEN,
	UI_BUTTON_COUNT
};

class EntityUI : public EntityMesh {

public:

	EntityUI(Vector2 size, const Material& material);
	EntityUI(Vector2 pos, Vector2 size, const Material& material, eUIButtonID button_id = UI_BUTTON_UNDEFINED, const std::string& name = "");
	~EntityUI() {};

	Vector2 position;
	Vector2 size;

	float mask = 1.0f;

	bool is3D = false;
	bool visible = true;
	Vector3 pos3d;

	eUIButtonID button_id = UI_BUTTON_UNDEFINED;

	void render(Camera* camera) override;
	void update(float seconds_elapsed) override;
	void update3D(const Vector3& position3d);
};