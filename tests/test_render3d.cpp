// =====================================================================
// GARUDA COMPILER — FASE 22 HIGH-PERFORMANCE 3D RENDER ENGINE SUITE (5 TESTS)
// =====================================================================

#include "../include/garuda/beam/game/render3d_engine.hpp"
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
    std::cout << "GARUDA COMPILER — FASE 22 HIGH-PERFORMANCE 3D RENDER ENGINE SUITE\n";
    std::cout << "=======================================================\n\n";

    // === Test 1: 3D Vector Math & Operations ===
    std::cout << "[TEST 1/5] Running 3D Vector Math test...\n";
    Vec3 v1{1.0, 2.0, 3.0};
    Vec3 v2{4.0, 5.0, 6.0};
    Vec3 v3 = v1 + v2;

    CHECK(v3.x == 5.0 && v3.y == 7.0 && v3.z == 9.0, "Vec3 addition must match");
    CHECK(v1.dot(v2) == 32.0, "Vec3 dot product must equal 32.0");
    std::cout << "  ✓ Test 1: 3D Vector Math PASS\n";

    // === Test 2: 3D Mesh Geometry Setup ===
    std::cout << "[TEST 2/5] Running 3D Mesh Geometry test...\n";
    Mesh3D cube;
    cube.name = "CubePrimitive";
    cube.vertices = {
        {{-1, -1, 1}, {0, 0, 1}, 0, 0},
        {{1, -1, 1}, {0, 0, 1}, 1, 0},
        {{1, 1, 1}, {0, 0, 1}, 1, 1}
    };
    cube.indices = {0, 1, 2};

    CHECK(cube.vertices.size() == 3, "Cube mesh must contain 3 vertices");
    CHECK(cube.indices.size() == 3, "Cube mesh must contain 3 indices");
    std::cout << "  ✓ Test 2: 3D Mesh Geometry PASS\n";

    // === Test 3: Camera 3D Position & Forward Vector ===
    std::cout << "[TEST 3/5] Running Camera 3D Position & Vector test...\n";
    Camera3D cam;
    cam.set_position({0.0, 0.0, 10.0});
    cam.set_target({0.0, 0.0, 0.0});

    Vec3 fwd = cam.forward();
    CHECK(fwd.z == -1.0, "Camera forward vector pointing at origin must be (0,0,-1)");
    std::cout << "  ✓ Test 3: Camera 3D Position & Vector PASS\n";

    // === Test 4: Deferred Render Pipeline Light Submission ===
    std::cout << "[TEST 4/5] Running Render Pipeline Lighting test...\n";
    Render3DPipeline pipeline;
    pipeline.add_light({{0.0, 10.0, 0.0}, {1.0, 1.0, 1.0}, 2.5});
    pipeline.add_light({{5.0, 2.0, -3.0}, {0.8, 0.8, 1.0}, 1.0});

    CHECK(pipeline.light_count() == 2, "Pipeline must register 2 lights");
    std::cout << "  ✓ Test 4: Render Pipeline Lighting PASS\n";

    // === Test 5: End-to-End 3D Frame Render Pass ===
    std::cout << "[TEST 5/5] Running End-to-End 3D Frame Render Pass test...\n";
    Material3D pbr_mat{"hero_albedo.png", 0.8, 0.2, 0xFF0000FF};
    pipeline.submit_mesh(cube, pbr_mat, {0.0, 0.0, 0.0});

    std::size_t triangles_rendered = pipeline.render_frame(cam);
    CHECK(triangles_rendered == 1, "Frame render pass must render 1 triangle");
    std::cout << "  ✓ Test 5: End-to-End 3D Frame Render Pass PASS\n";

    std::cout << "\n========================================\n";
    std::cout << "🎉 ALL 5 3D RENDER ENGINE TESTS PASSED PERFECTLY!\n";
    std::cout << "========================================\n";
    return 0;
}
