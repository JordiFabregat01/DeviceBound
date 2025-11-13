#include "ball.h"
#include "game/world.h"
#include "game/player.h"
#include "game/game.h"

#include "framework/framework.h"

Ball::Ball(Mesh* mesh, const Material& material, const Vector3& position, const Vector3& initial_velocity)
    : EntityMesh(mesh, material, "Ball"), lifetime(4.0f), collisionCooldown(0.0f) {
    this->model.setTranslation(position);
    this->velocity = initial_velocity;
}
void Ball::update(float delta_time) {
    // Reduce lifetime
    lifetime -= delta_time;

    if (lifetime <= 0.0f) {
        World::get_instance()->destroyEntity(this);
        return;
    }

    // Apply gravity
    velocity.y -= 9.8f * delta_time;

    // Predict new position
    Vector3 old_position = model.getTranslation();
    Vector3 new_position = old_position + velocity * delta_time * 0.7f;

    // Check for collisions
    std::vector<sCollisionData> collisions;
    std::vector<sCollisionData> ground_collisions;
    World::get_instance()->test_scene_collisions(new_position, collisions, ground_collisions);

    bool is_grounded = false;
    Vector3 teleport_position;

    // Ground Collision Handling
    for (const sCollisionData& collision : ground_collisions) {
        if (fabsf(collision.col_normal.y) > 0.8f) { 
            is_grounded = true;
            teleport_position = collision.col_point;
        }
    }

    // If the ball hits the ground, notify the player and destroy itself
    if (is_grounded) {
		background_music_channel = Game::instance->audio.Play("data/audio/tp.mp3", 0.01f, true);
        World::get_instance()->set_last_ball_collision(teleport_position);
        World::get_instance()->destroyEntity(this);
        return;
    }

    for (const sCollisionData& collision : collisions) {
        float up_factor = fabsf(collision.col_normal.y);
        if (up_factor > 0.8f) continue;

        Vector3 reflection = velocity - 2 * velocity.dot(collision.col_normal) * collision.col_normal;
        reflection.y = velocity.y; 

        velocity = reflection;

        new_position = collision.col_point + collision.col_normal * 0.5f;
    }

    if (velocity.length() < 0.5f) {
        velocity = Vector3(0, 0, 0);
    }

    // Update the ball's position
    model.setTranslation(new_position);
}