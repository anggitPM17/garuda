// =====================================================================
// GARUDA COMPILER — FASE 23 3D SCENE GRAPH ENGINE SUITE (5 TESTS)
// =====================================================================

#include "../include/garuda/beam/game/scene_graph.hpp"
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
    std::cout << "GARUDA COMPILER — FASE 23 3D SCENE GRAPH ENGINE SUITE\n";
    std::cout << "=======================================================\n\n";

    // === Test 1: Scene Hierarchy Setup & Parent-Child Tree ===
    std::cout << "[TEST 1/5] Running Scene Hierarchy Setup test...\n";
    Scene scene;
    SceneNode* root = scene.root();
    CHECK(root->name() == "RootNode", "Root node name must be RootNode");

    auto child1 = std::make_unique<SceneNode>("CarParent");
    SceneNode* car = root->add_child(std::move(child1));
    CHECK(root->child_count() == 1, "Root must contain 1 child node");

    auto wheel1 = std::make_unique<SceneNode>("WheelFrontLeft");
    car->add_child(std::move(wheel1));
    CHECK(car->child_count() == 1, "Car node must contain 1 wheel child node");
    std::cout << "  ✓ Test 1: Scene Hierarchy Setup PASS\n";

    // === Test 2: Spatial World Transform Matrix Calculation ===
    std::cout << "[TEST 2/5] Running Spatial World Transform Calculation test...\n";
    car->set_local_position({10.0, 0.0, 0.0});
    auto wheel_node = std::make_unique<SceneNode>("WheelFrontRight");
    wheel_node->set_local_position({1.5, 0.5, 2.0});
    SceneNode* wheel_ptr = car->add_child(std::move(wheel_node));

    scene.update(0.016);
    Transform3D w_tf = wheel_ptr->world_transform();
    CHECK(w_tf.position.x == 11.5 && w_tf.position.y == 0.5 && w_tf.position.z == 2.0,
          "Child world position must equal parent position + child local position");
    std::cout << "  ✓ Test 2: Spatial World Transform Calculation PASS\n";

    // === Test 3: Scene Graph Traversal Visitor Pattern ===
    std::cout << "[TEST 3/5] Running Scene Graph Traversal Visitor test...\n";
    std::size_t visited_nodes = 0;
    root->traverse([&visited_nodes](SceneNode& node) {
        visited_nodes++;
    });

    CHECK(visited_nodes == 4, "Traversal visitor must visit exactly 4 nodes in scene tree");
    std::cout << "  ✓ Test 3: Scene Graph Traversal Visitor PASS\n";

    // === Test 4: Component Attachments (Mesh & Material) ===
    std::cout << "[TEST 4/5] Running Component Attachments test...\n";
    Mesh3D chassis_mesh;
    chassis_mesh.name = "ChassisMesh";
    chassis_mesh.vertices = {{{0,0,0}, {0,1,0}, 0, 0}, {{1,0,0}, {0,1,0}, 1, 0}, {{0,1,0}, {0,1,0}, 0, 1}};
    chassis_mesh.indices = {0, 1, 2};

    car->mesh = chassis_mesh;
    car->material = Material3D{"metal.png", 0.9, 0.1, 0xFF0000FF};

    CHECK(car->mesh.has_value() && car->material.has_value(), "Car node must have mesh & material components");
    std::cout << "  ✓ Test 4: Component Attachments PASS\n";

    // === Test 5: Automatic Scene Rendering Pass ===
    std::cout << "[TEST 5/5] Running Automatic Scene Rendering Pass test...\n";
    scene.active_camera().set_position({0.0, 0.0, 20.0});
    scene.active_camera().set_target({0.0, 0.0, 0.0});
    scene.add_light({{0.0, 10.0, 0.0}, {1.0, 1.0, 1.0}, 2.0});

    Render3DPipeline pipeline;
    std::size_t rendered_tris = scene.render(pipeline);

    CHECK(rendered_tris == 1, "Scene render pass must render 1 mesh triangle");
    CHECK(pipeline.light_count() == 1, "Pipeline must register 1 light from scene");
    std::cout << "  ✓ Test 5: Automatic Scene Rendering Pass PASS\n";

    std::cout << "\n========================================\n";
    std::cout << "🎉 ALL 5 3D SCENE GRAPH ENGINE TESTS PASSED PERFECTLY!\n";
    std::cout << "========================================\n";
    return 0;
}
