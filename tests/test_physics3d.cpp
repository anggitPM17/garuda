// =====================================================================
// GARUDA COMPILER — FASE 22 3D PHYSICS & SPATIAL AUDIO SUITE (5 TESTS)
// =====================================================================

#include "../include/garuda/beam/game/physics3d_engine.hpp"
#include <cassert>
#include <cmath>
#include <iostream>

using namespace garuda::beam::game;

#define CHECK(cond, msg) do { \
    if (!(cond)) { \
        std::cerr << "[FAIL] " << msg << "\n"; \
        return 1; \
    } \
} while(0)

int main() {
    std::cout << "=======================================================\n";
    std::cout << "GARUDA COMPILER — FASE 22 3D PHYSICS & SPATIAL AUDIO SUITE\n";
    std::cout << "=======================================================\n\n";

    // === Test 1: 3D RigidBody Gravity & Velocity Integration ===
    std::cout << "[TEST 1/5] Running 3D RigidBody Integration test...\n";
    std::vector<RigidBody3D> bodies;
    std::vector<Collider3D> colliders;

    bodies.push_back(RigidBody3D{{0.0, 10.0, 0.0}, {0.0, 0.0, 0.0}, 1.0, 1.0, false});
    colliders.push_back(Collider3D{{1.0, 1.0, 1.0}, false});

    PhysicsEngine3D physics;
    physics.update(bodies, colliders, 0.1); // 0.1 sec step

    CHECK(bodies[0].position.y < 10.0, "RigidBody Y position must drop due to gravity");
    CHECK(bodies[0].velocity.y < 0.0, "RigidBody Y velocity must be negative");
    std::cout << "  ✓ Test 1: 3D RigidBody Integration PASS\n";

    // === Test 2: Static 3D Object Motion Immunity ===
    std::cout << "[TEST 2/5] Running Static 3D Object Motion Immunity test...\n";
    bodies.push_back(RigidBody3D{{0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, 10.0, 1.0, true}); // Static floor
    colliders.push_back(Collider3D{{10.0, 1.0, 10.0}, false});

    physics.update(bodies, colliders, 0.1);
    CHECK(bodies[1].position.y == 0.0, "Static RigidBody position must remain fixed");
    std::cout << "  ✓ Test 2: Static 3D Object Motion Immunity PASS\n";

    // === Test 3: 3D AABB Collision Detection ===
    std::cout << "[TEST 3/5] Running 3D AABB Collision Detection test...\n";
    bodies[0].position = {0.0, 0.5, 0.0}; // Overlapping with floor at (0,0,0)
    physics.update(bodies, colliders, 0.01);

    CHECK(colliders[0].is_colliding && colliders[1].is_colliding, "3D AABB boxes must trigger collision");
    std::cout << "  ✓ Test 3: 3D AABB Collision Detection PASS\n";

    // === Test 4: Spatial Audio 3D Distance Attenuation ===
    std::cout << "[TEST 4/5] Running Spatial Audio 3D Distance Attenuation test...\n";
    SpatialAudio3D audio;
    audio.set_listener_position({0.0, 0.0, 0.0});

    SpatialAudio3D::AudioEmitter near_emitter{"explosion.wav", {0.0, 0.0, 10.0}, 1.0, 100.0};
    double vol_near = audio.calculate_perceived_volume(near_emitter);
    CHECK(vol_near > 0.8 && vol_near < 1.0, "Near emitter perceived volume must be ~0.9");

    SpatialAudio3D::AudioEmitter far_emitter{"explosion.wav", {0.0, 0.0, 90.0}, 1.0, 100.0};
    double vol_far = audio.calculate_perceived_volume(far_emitter);
    CHECK(vol_far < 0.2, "Far emitter perceived volume must be attenuated");
    CHECK(vol_near > vol_far, "Near emitter volume must be louder than far emitter");
    std::cout << "  ✓ Test 4: Spatial Audio 3D Distance Attenuation PASS\n";

    // === Test 5: Out-of-Range Spatial Audio Cutoff ===
    std::cout << "[TEST 5/5] Running Out-of-Range Audio Cutoff test...\n";
    SpatialAudio3D::AudioEmitter out_emitter{"explosion.wav", {0.0, 0.0, 150.0}, 1.0, 100.0};
    double vol_out = audio.calculate_perceived_volume(out_emitter);

    CHECK(vol_out == 0.0, "Out-of-range emitter volume must be cutoff to 0.0");
    std::cout << "  ✓ Test 5: Out-of-Range Audio Cutoff PASS\n";

    std::cout << "\n========================================\n";
    std::cout << "🎉 ALL 5 3D PHYSICS & SPATIAL AUDIO TESTS PASSED PERFECTLY!\n";
    std::cout << "========================================\n";
    return 0;
}
