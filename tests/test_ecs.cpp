// =====================================================================
// GARUDA COMPILER — FASE 19 GAME ENGINE & ECS SUITE (5 TESTS)
// =====================================================================

#include "../include/garuda/beam/game/game_engine.hpp"
#include <cassert>
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
    std::cout << "GARUDA COMPILER — FASE 19 GAME ENGINE & ECS SUITE\n";
    std::cout << "=======================================================\n\n";

    // === Test 1: ECS Entity Creation & Component Binding ===
    std::cout << "[TEST 1/5] Running ECS Entity Creation & Component Binding test...\n";
    EcsRegistry registry;
    Entity player = registry.create_entity();
    CHECK(registry.is_alive(player), "Player entity must be alive");
    CHECK(registry.entity_count() == 1, "Entity count must be 1");

    registry.add_component(player, TransformComponent{10.0, 20.0, 0.0});
    registry.add_component(player, RigidBodyComponent{5.0, 0.0, 1.0, 1.0, false});

    CHECK(registry.has_component<TransformComponent>(player), "Player must have TransformComponent");
    CHECK(registry.has_component<RigidBodyComponent>(player), "Player must have RigidBodyComponent");

    auto* tf = registry.get_component<TransformComponent>(player);
    CHECK(tf != nullptr && tf->x == 10.0 && tf->y == 20.0, "TransformComponent coordinates must match");
    std::cout << "  ✓ Test 1: ECS Entity Creation & Component Binding PASS\n";

    // === Test 2: ECS Entity Destruction & Component Cleanup ===
    std::cout << "[TEST 2/5] Running ECS Entity Destruction test...\n";
    Entity temp_enemy = registry.create_entity();
    registry.add_component(temp_enemy, TransformComponent{50.0, 50.0, 0.0});
    CHECK(registry.entity_count() == 2, "Entity count must be 2");

    registry.destroy_entity(temp_enemy);
    CHECK(!registry.is_alive(temp_enemy), "Destroyed entity must not be alive");
    CHECK(!registry.has_component<TransformComponent>(temp_enemy), "Destroyed entity components must be removed");
    CHECK(registry.entity_count() == 1, "Entity count must decrease to 1");
    std::cout << "  ✓ Test 2: ECS Entity Destruction PASS\n";

    // === Test 3: Physics 2D AABB Collision Engine ===
    std::cout << "[TEST 3/5] Running Physics 2D AABB Collision test...\n";
    Entity box_a = registry.create_entity();
    Entity box_b = registry.create_entity();

    registry.add_component(box_a, TransformComponent{0.0, 0.0, 0.0});
    registry.add_component(box_a, ColliderComponent{32.0, 32.0, false, false});

    registry.add_component(box_b, TransformComponent{16.0, 16.0, 0.0});
    registry.add_component(box_b, ColliderComponent{32.0, 32.0, false, false});

    PhysicsEngine2D physics;
    physics.update(registry, 0.016);

    auto* c_a = registry.get_component<ColliderComponent>(box_a);
    auto* c_b = registry.get_component<ColliderComponent>(box_b);
    CHECK(c_a->is_colliding && c_b->is_colliding, "Overlapping AABB boxes must trigger collision");
    std::cout << "  ✓ Test 3: Physics 2D AABB Collision PASS\n";

    // === Test 4: Input Engine Keyboard & Mouse Management ===
    std::cout << "[TEST 4/5] Running Input Engine test...\n";
    InputEngine input;
    input.press_key("SPACE");
    input.press_key("KEY_W");
    input.set_mouse_pos(192.0, 108.0);

    CHECK(input.is_key_pressed("SPACE"), "SPACE key must be pressed");
    CHECK(input.is_key_pressed("KEY_W"), "KEY_W key must be pressed");
    CHECK(!input.is_key_pressed("KEY_S"), "KEY_S key must not be pressed");
    CHECK(input.mouse_x() == 192.0 && input.mouse_y() == 108.0, "Mouse position must match");

    input.release_key("SPACE");
    CHECK(!input.is_key_pressed("SPACE"), "SPACE key must be released");
    std::cout << "  ✓ Test 4: Input Engine PASS\n";

    // === Test 5: Render Pipeline Sprite Batch Recorder ===
    std::cout << "[TEST 5/5] Running Render Pipeline test...\n";
    Entity sprite1 = registry.create_entity();
    registry.add_component(sprite1, TransformComponent{100.0, 200.0, 0.0, 0.0, 2.0, 2.0});
    registry.add_component(sprite1, RenderSpriteComponent{"tex_hero.png", 0xFFFFFFFF, 1});

    RenderPipeline renderer;
    renderer.record_frame(registry);

    CHECK(renderer.draw_call_count() > 0, "RenderPipeline must record draw calls for sprite entities");
    const auto& calls = renderer.draw_calls();
    CHECK(calls[0].sprite_id == "tex_hero.png", "Sprite ID must match");
    CHECK(calls[0].x == 100.0 && calls[0].y == 200.0, "Render coordinates must match TransformComponent");
    std::cout << "  ✓ Test 5: Render Pipeline PASS\n";

    std::cout << "\n========================================\n";
    std::cout << "🎉 ALL 5 GAME ENGINE & ECS TESTS PASSED PERFECTLY!\n";
    std::cout << "========================================\n";
    return 0;
}
