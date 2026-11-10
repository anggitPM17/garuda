#pragma once
// Garuda Standard Library: 3D Physics & Positional Spatial Audio Engine (`physics3d_engine.hpp`)
// Mengimplementasikan RigidBody 3D, AABB 3D Collision, & Spatial Audio 3D Distance Attenuation.

#include "render3d_engine.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

namespace garuda::beam::game {

struct RigidBody3D {
    Vec3 position;
    Vec3 velocity;
    double mass = 1.0;
    double gravity_scale = 1.0;
    bool is_static = false;
};

struct Collider3D {
    Vec3 extents{1.0, 1.0, 1.0}; // Half-widths along x, y, z
    bool is_colliding = false;
};

class PhysicsEngine3D {
public:
    PhysicsEngine3D() = default;

    void update(std::vector<RigidBody3D>& bodies, std::vector<Collider3D>& colliders, double dt) {
        Vec3 gravity{0.0, -9.8, 0.0};

        // 1. Velocity & Position Integration
        for (auto& body : bodies) {
            if (body.is_static) continue;
            body.velocity = body.velocity + (gravity * body.gravity_scale * dt);
            body.position = body.position + (body.velocity * dt);
        }

        // 2. 3D AABB Collision Detection
        std::size_t n = std::min(bodies.size(), colliders.size());
        for (std::size_t i = 0; i < n; ++i) {
            for (std::size_t j = i + 1; j < n; ++j) {
                bool hit = check_aabb3d(bodies[i].position, colliders[i].extents,
                                       bodies[j].position, colliders[j].extents);
                if (hit) {
                    colliders[i].is_colliding = true;
                    colliders[j].is_colliding = true;
                }
            }
        }
    }

    static bool check_aabb3d(const Vec3& p1, const Vec3& e1, const Vec3& p2, const Vec3& e2) {
        return (std::abs(p1.x - p2.x) <= (e1.x + e2.x) &&
                std::abs(p1.y - p2.y) <= (e1.y + e2.y) &&
                std::abs(p1.z - p2.z) <= (e1.z + e2.z));
    }
};

class SpatialAudio3D {
public:
    SpatialAudio3D() : listener_pos_{0, 0, 0} {}

    void set_listener_position(Vec3 pos) {
        listener_pos_ = pos;
    }

    struct AudioEmitter {
        std::string sound_id;
        Vec3 position;
        double volume = 1.0;
        double max_distance = 100.0;
    };

    double calculate_perceived_volume(const AudioEmitter& emitter) const {
        double dist = (emitter.position - listener_pos_).length();
        if (dist >= emitter.max_distance) return 0.0;
        // Inverse distance attenuation curve
        double attenuation = 1.0 - (dist / emitter.max_distance);
        return std::max(0.0, emitter.volume * attenuation);
    }

private:
    Vec3 listener_pos_;
};

} // namespace garuda::beam::game
