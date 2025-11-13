#pragma once

#ifndef BALL_H
#define BALL_H

#include "entity_mesh.h"
#include "framework/audio.h"

class Ball : public EntityMesh {
    HCHANNEL background_music_channel;

public:
    Vector3 velocity;
    float lifetime;         // Time before the ball is destroyed
    float collisionCooldown; // Timer to prevent multiple wall bounces

    Ball(Mesh* mesh, const Material& material, const Vector3& position, const Vector3& initial_velocity);

    void update(float delta_time) override;
};

#endif