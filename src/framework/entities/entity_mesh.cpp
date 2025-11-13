#include "entity_mesh.h"
#include "framework/camera.h"
#include "graphics/mesh.h"
#include "graphics/shader.h"
#include "graphics/material.h"
#include "graphics/texture.h"
#include "game/world.h"
#include "game/game.h"
#include "framework/input.h"
#include "game/stage.h"
#include "game/player.h"
#include "game/scene_parser.h"


EntityMesh::EntityMesh(Mesh* new_mesh, const Material& new_material)
{
	mesh = new_mesh;
	material = new_material;
}

EntityMesh::EntityMesh(Mesh* new_mesh, const Material& new_material, const std::string& new_name)
{
	mesh = new_mesh;
	material = new_material;
	name = new_name;
}

void EntityMesh::render(Camera* camera)
{
	if (!mesh) return;

	bool must_render = true;

	std::vector<Matrix44> must_render_models;

	if (!isInstanced) {
		Matrix44 global_matrix = getGlobalMatrix();
		float distance = camera->eye.distance(global_matrix.getTranslation());
		must_render &= (distance < distance_culling);

		Vector3 bb_center = global_matrix * mesh->box.center;
		Vector3 bb_halfsize = mesh->box.halfsize * 25.f;
		char r = camera->testBoxInFrustum(bb_center, bb_halfsize);
		must_render &= (r != CLIP_OUTSIDE);
	}
	else {
		for (const Matrix44& model : models) {
			Matrix44 global_matrix = model * parent->getGlobalMatrix();
			float distance = camera->eye.distance(global_matrix.getTranslation());
			Vector3 bb_center = global_matrix * mesh->box.center;
			Vector3 bb_halfsize = mesh->box.halfsize * 25.f;
			if (distance < distance_culling && camera->testBoxInFrustum(bb_center, bb_halfsize) != CLIP_OUTSIDE)
				must_render_models.push_back(model);
		}
	}

	if (!must_render) {
		return;
	}
	if (!material.shader) {
		material.shader = Shader::Get(isInstanced ? "data/shaders/instanced.vs" : "data/shaders/basic.vs", "data/shaders/phong.fs");
	}

	camera->enable();

	Shader* shader = material.shader;

	shader->enable();

	shader->setUniform("u_color", material.color);
	shader->setUniform("u_viewprojection", camera->viewprojection_matrix);
	shader->setUniform("u_background_color", Vector4(0.1f, 0.1f, 0.1f, 1.f));	
	shader->setUniform("u_camera_position", camera->eye);

	shader->setUniform("u_Ka", Vector3(1.0f));
	shader->setUniform("u_light_color", Vector3(0.9f, 0.9f, 1.0f));
	shader->setUniform("u_light_position", Vector3(0.0f, 30.0f, 20.0f));
	shader->setUniform("u_fog_factor", 0.8f);
	shader->setUniform("u_time", Game::instance->time);

	Vector2 maps = { 0.0f, 0.0f };

	Texture* cube_texture = new Texture();
	cube_texture = nullptr;
	cube_texture = Texture::Get("landscape");

	if (cube_texture != nullptr) {
		shader->setUniform("u_camera_position", camera->eye);
	}

	if (material.diffuse) {
		maps.x = 1.0f;
		shader->setUniform("u_texture", material.diffuse, 0);
	}
	if (material.normal) {
		maps.y = 1.0f;
		shader->setUniform("u_texture_normal", material.normal, 1);
	}

	material.shader->setUniform("u_texture_cube", Texture::Get("landscape"));

	material.shader->setUniform("u_maps", maps);

	if (isInstanced) {
		mesh->renderInstanced(GL_TRIANGLES, must_render_models.data(), must_render_models.size());
	}
	else {
		shader->setUniform("u_model", getGlobalMatrix());
		mesh->render(GL_TRIANGLES);
	}

	shader->disable();

	Entity::render(camera);
}

void EntityMesh::update(float delta_time)
{
	Entity::update(delta_time);
}
void EntityMesh::addInstance(const Matrix44& model)
{
	models.push_back(model);
}
void EntityCollider::getCollisionsWithModel(const Matrix44& m, const Vector3& center, std::vector<sCollisionData>& collisions, std::vector<sCollisionData>& ground_collisions) {
	/*
	Vector3 col_point;
	Vector3 col_normal;
	float distance = 3.4e+38F;
	bool collided = false;
	EntityCollider* collider = nullptr;
	*/
	Vector3 collision_point;
	Vector3 collision_normal;

	float sphere_radius = 0.25f;
	float sphere_ground_radius = 0.15f;
	float player_height = 0.35f;


	Vector3 floor_sphere_center = center + Vector3(0.0f, sphere_ground_radius, 0.0f);

	if (mesh->testSphereCollision(m, floor_sphere_center, sphere_ground_radius, collision_point, collision_normal)) {
		sCollisionData data;
		data.col_point = collision_point;
		data.col_normal = collision_normal.normalize();
		data.collided = true;
		data.collider = this;
		data.distance = (floor_sphere_center - collision_point).length();
		collisions.push_back(data);
		//collisions.push_back({collision_point, collision_normal.normalize(), floor_sphere_center, true, this});
	}

	Vector3 character_center = center + Vector3(0.f, player_height, 0.f);
	if (mesh->testSphereCollision(m, character_center, sphere_radius, collision_point, collision_normal)) {
		sCollisionData data;
		data.col_point = collision_point;
		data.col_normal = collision_normal.normalize();
		data.collided = true;
		data.collider = this;
		data.distance = (character_center - collision_point).length();
		collisions.push_back(data);
		//collisions.push_back({ collision_point, collision_normal.normalize(), character_center });
	}

	if (mesh->testRayCollision(m, character_center, Vector3(0, -1, 0), collision_point, collision_normal, player_height + 0.01f)) {
		sCollisionData data;
		data.col_point = collision_point;
		data.col_normal = collision_normal.normalize();
		data.collided = true;
		data.collider = this;
		data.distance = (character_center - collision_point).length();
		ground_collisions.push_back(data);
		//ground_collisions.push_back({ collision_point, collision_normal.normalize(), character_center });
	}
	// Check for collision at the top of the character
	if (mesh->testRayCollision(m, character_center, Vector3(0, 1, 0), collision_point, collision_normal, player_height + 0.01f)) {
		sCollisionData data;
		data.col_point = collision_point;
		data.col_normal = collision_normal.normalize();
		data.collided = true;
		data.collider = this;
		data.distance = (character_center - collision_point).length();
		collisions.push_back(data);
		//collisions.push_back({ collision_point, collision_normal.normalize(), character_center });
	}
}
void EntityCollider::getCollisions(const Vector3& target_position, std::vector<sCollisionData>& collisions, std::vector<sCollisionData>& ground_collisions)
{
	/*if (!(layer & filter))
		return;*/
	if (!isInstanced) {
		getCollisionsWithModel(model, target_position, collisions, ground_collisions);
	}
	else {
		for (int i = 0; i < models.size(); ++i) {
			getCollisionsWithModel(models[i], target_position, collisions, ground_collisions);
		}
	}
	return;
}
EntityUI::EntityUI(Vector2 new_size, const Material& material) {
	size = new_size;
	this->material = material;

	if (!this->material.shader) {
		this->material.shader = Shader::Get("data/shaders/basic.vs", material.diffuse ? "data/shaders/texture.fs" : "data/shaders/flat.fs");
	}
}

EntityUI::EntityUI(Vector2 new_pos, Vector2 new_size, const Material& material, eUIButtonID new_button_id, const std::string& new_name) {
	position = new_pos;
	size = new_size;
	button_id = new_button_id;
	mesh = new Mesh();
	mesh->createQuad(position.x, position.y, size.x, size.y, true);

	this->material = material;

	if (!this->material.shader) {
		this->material.shader = Shader::Get("data/shaders/basic.vs", material.diffuse ? "data/shaders/texture.fs" : "data/shaders/flat.fs");
	}
}

void EntityUI::render(Camera* camera2d) {
	if (!visible)
		return;

	if (!is3D) {
		glDisable(GL_DEPTH_TEST);
	}

	glDisable(GL_CULL_FACE);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	material.shader->enable();

	Matrix44 viewProj = camera2d->viewprojection_matrix;

	material.shader->setUniform("u_model", model);
	material.shader->setUniform("u_viewprojection", viewProj);
	material.shader->setUniform("u_color", material.color);
	material.shader->setUniform("u_mask", mask);

	if (material.diffuse) {
		material.shader->setUniform("u_texture", material.diffuse, 0);
	}

	if (is3D) {
		Mesh quad;
		quad.createQuad(position.x, position.y, size.x, size.y, true);
		quad.render(GL_TRIANGLES);
	}
	else {
		mesh->render(GL_TRIANGLES);
	}
	material.shader->disable();

	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);

	Entity::render(camera2d);
}

void EntityUI::update(float seconds_elapsed) {

	Vector2 mouse_pos = Input::mouse_position;

	if (button_id != UI_BUTTON_UNDEFINED &&
		mouse_pos.x > (position.x - size.x * 0.5f) &&
		mouse_pos.x < (position.x + size.x * 0.5f) &&
		mouse_pos.y >(position.y - size.y * 0.5f) &&
		mouse_pos.y < (position.y + size.y * 0.5f)) {

		material.color = Vector4(2.0f);

		if (Input::wasMousePressed(SDL_BUTTON_LEFT) && button_id == UI_BUTTON_PLAY) {
			Game::instance->goToStage(STAGE_PLAY);
		}
		else if (Input::wasMousePressed(SDL_BUTTON_LEFT) && button_id == UI_BUTTON_EXIT) {
			exit(0);
		}
		else if (Input::wasMousePressed(SDL_BUTTON_LEFT) && button_id == UI_BUTTON_OPTIONS) {
			Game::instance->goToStage(STAGE_OPTIONS);
		}
		else if (Input::wasMousePressed(SDL_BUTTON_LEFT) && button_id == UI_BUTTON_BACK) {
			Game::instance->goToStage(STAGE_MENU);
		}
		else if (Input::wasMousePressed(SDL_BUTTON_LEFT) && button_id == UI_BUTTON_INCREASE_SENSITIVITY) {
			if (World::get_instance()->mouse_speed < 1.0f) {
				World::get_instance()->mouse_speed += 0.05f;
			}
		}
		else if (Input::wasMousePressed(SDL_BUTTON_LEFT) && button_id == UI_BUTTON_DECREASE_SENSITIVITY) {
			if (World::get_instance()->mouse_speed > 0.06f) {
				World::get_instance()->mouse_speed -= 0.05f;
			}
		}
	}
	else {
		material.color = Vector4::WHITE;
	}
	Entity::update(seconds_elapsed);
}

void EntityUI::update3D(const Vector3& position3d) {
	pos3d = position3d;

	int width = Game::instance->window_width;
	int height = Game::instance->window_height;
	World* world = World::get_instance();

	visible = true;

	Vector3 pos = world->camera->project(position3d, width, height);
	if (pos.z < -1.0f || pos.z > 1.0f) {
		visible = false;
	}
	else {
		pos.y = height - pos.y;
		position = Vector2(pos.x, pos.y);
	}
}