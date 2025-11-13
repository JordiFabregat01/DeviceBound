
#include "world.h"
#include "game.h"
#include "framework/utils.h"
#include "graphics/mesh.h"
#include "graphics/texture.h"
#include "graphics/fbo.h"
#include "graphics/shader.h"
#include "framework/input.h"
#include "scene_parser.h"
#include <cmath>
#include "framework/entities/entity_mesh.h"
#include "stage.h"
#include "player.h"


World* World::instance = nullptr;

World::World() {
	int window_width = Game::instance->window_width;
	int window_height = Game::instance->window_height;

	camera = new Camera();
	camera->lookAt(Vector3(0.f, 1.f, 1.f), Vector3(0.f, 0.f, 0.f), Vector3(0.f, 1.f, 0.f));
	camera->setPerspective(70.f, window_width / (float)window_height, 0.01f, 1000.f);

	camera2D = new Camera();
	camera2D->view_matrix.setIdentity();
	camera2D->setOrthographic(0, window_width, window_height, 0, -1, 1);

	root = new Entity();

	{
		Texture* cube_texture = new Texture();
		cube_texture->loadCubemap("landscape", {
			"data/textures/skybox/px.png",
			"data/textures/skybox/nx.png",
			"data/textures/skybox/ny.png",
			"data/textures/skybox/py.png",
			"data/textures/skybox/pz.png",
			"data/textures/skybox/nz.png"
			});

		Material cubemap_material;
		cubemap_material.shader = Shader::Get("data/shaders/basic.vs", "data/shaders/cubemap.fs");
		cubemap_material.diffuse = cube_texture;

		skybox = new EntityMesh(Mesh::Get("data/meshes/cubemap.ASE"), cubemap_material);
	}

	Material player_material;
	player_material.diffuse = Texture::Get("data/meshes/colormap.png");
	player_material.shader = Shader::Get("data/shaders/basic.vs", "data/shaders/texture.fs");

	player = new Player(Mesh::Get("data/meshes/soldier.obj"), player_material);
	player->model.setTranslation(Vector3(-5.0f, 0.5f, -3.0f)); // Set initial position
	root->addChild(player);

	SceneParser parser;
	bool ok = parser.parse("data/myscene.scene", root);
	assert(ok);
}

void World::render()
{
	camera->enable();

	glDisable(GL_BLEND);
	glDisable(GL_CULL_FACE);

	glDisable(GL_DEPTH_TEST);
	skybox->render(camera);
	glEnable(GL_DEPTH_TEST);

	//drawGrid();

	if (camera_type != CAMERA_FIRST_PERSON)
	{
		player->render(camera);
	}

	root->render(camera);
}

void World::update(double seconds_elapsed)
{
	if (free_camera)
	{
		float speed = (float)seconds_elapsed * camera_speed; //the speed is defined by the seconds_elapsed so it goes constant

		// Mouse input to rotate the cam
		if (Input::isMousePressed(SDL_BUTTON_LEFT) || Game::instance->mouse_locked) //is left button pressed?
		{
			camera->rotate(Input::mouse_delta.x * 0.005f, Vector3(0.0f, -1.0f, 0.0f));
			camera->rotate(Input::mouse_delta.y * 0.005f, camera->getLocalVector(Vector3(-1.0f, 0.0f, 0.0f)));
		}

		// Async input to move the camera around
		if (Input::isKeyPressed(SDL_SCANCODE_LSHIFT)) speed *= 10; //move faster with left shift
		if (Input::isKeyPressed(SDL_SCANCODE_W) || Input::isKeyPressed(SDL_SCANCODE_UP)) camera->move(Vector3(0.0f, 0.0f, 1.0f) * speed);
		if (Input::isKeyPressed(SDL_SCANCODE_S) || Input::isKeyPressed(SDL_SCANCODE_DOWN)) camera->move(Vector3(0.0f, 0.0f, -1.0f) * speed);
		if (Input::isKeyPressed(SDL_SCANCODE_A) || Input::isKeyPressed(SDL_SCANCODE_LEFT)) camera->move(Vector3(1.0f, 0.0f, 0.0f) * speed);
		if (Input::isKeyPressed(SDL_SCANCODE_D) || Input::isKeyPressed(SDL_SCANCODE_RIGHT)) camera->move(Vector3(-1.0f, 0.0f, 0.0f) * speed);
	}
	else
	{

		root->update(seconds_elapsed);

		player->update(seconds_elapsed);

		switch (camera_type)
		{
		case CAMERA_FIRST_PERSON: update_fp_camera(seconds_elapsed); break;
		default: break;
		}
	}
	skybox->model.setTranslation(camera->eye);

	for (auto e : entities_to_destroy) {
		root->removeChild(e);
		delete e;
	}
	entities_to_destroy.clear();
}

void World::update_fp_camera(float seconds_elapsed) {

	camera_yaw -= Input::mouse_delta.x * seconds_elapsed * mouse_speed;
	camera_pitch -= Input::mouse_delta.y * seconds_elapsed * mouse_speed;

	camera_pitch = clamp(camera_pitch, -(float)M_PI * 0.4f, (float)M_PI * 0.4f);

	Matrix44 mYaw;
	mYaw.setRotation(camera_yaw, Vector3(0, 1, 0));

	Matrix44 mPitch;
	mPitch.setRotation(camera_pitch, Vector3(-1, 0, 0));

	Vector3 front = (mPitch * mYaw).frontVector().normalize();
	Vector3 eye = player->model.getTranslation() + Vector3(0.0f, player->player_height + player->bobbing_offset, 0.0f);
	Vector3 center = eye + front;

	camera->lookAt(eye, center, Vector3(0, 1, 0));
}

sCollisionData World::raycast(const Vector3& origin, const Vector3& direction, int layer, bool closest, float max_ray_distance)
{
	sCollisionData data;
	for (auto e : root->children) {
		EntityCollider* ec = dynamic_cast<EntityCollider*>(e);
		if (ec == nullptr || !(ec->layer & layer)) {
			continue;
		}

		Vector3 col_point;
		Vector3 col_normal;

		if (!ec->isInstanced) {
			if (!ec->mesh->testRayCollision(ec->model, origin, direction, col_point, col_normal)) {
				continue;
			}
			float new_distance = (col_point - origin).length();
			if (new_distance < data.distance) {
				data = { col_point, col_normal, new_distance, true, ec };
			}
			if (!closest) {
				return data;
			}
		}
		else
		{
			for (const Matrix44& model : ec->models) {
				if (!ec->mesh->testRayCollision(model, origin, direction, col_point, col_normal)) {
					continue;
				}
				float new_distance = (col_point - origin).length();
				if (new_distance < data.distance) {
					data = { col_point, col_normal, new_distance, true, ec };
				}
				if (!closest) {
					return data;
				}
			}
		}
	}
	return data;
}

void World::test_scene_collisions(const Vector3& position, std::vector<sCollisionData>& collisions, std::vector<sCollisionData>& ground_collisions)
{
	for (auto e : root->children) {
		EntityCollider* ec = dynamic_cast<EntityCollider*>(e);
		if (ec == nullptr) {
			continue;
		}
		ec->getCollisions(position, collisions, ground_collisions);
	}
}


void World::addEntity(Entity* entity)
{
	root->addChild(entity);
}

void World::destroyEntity(Entity* entity)
{
	entities_to_destroy.push_back(entity);
}
