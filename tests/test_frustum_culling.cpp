// =====================================================================
// GARUDA COMPILER — 3D CAMERA FRUSTUM CULLING SUITE (5 TESTS)
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
    std::cout << "GARUDA COMPILER — 3D CAMERA FRUSTUM CULLING SUITE\n";
    std::cout << "=======================================================\n\n";

    Camera3D cam;
    cam.set_position({0.0, 0.0, 10.0});
    cam.set_target({0.0, 0.0, 0.0});

    // === Test 1: Object in Front of Camera (Visible) ===
    std::cout << "[TEST 1/5] Running Object in Front of Camera test...\n";
    Vec3 front_obj{0.0, 0.0, 5.0};
    CHECK(cam.is_visible(front_obj, 1.0), "Object in front of camera must be visible");
    std::cout << "  ✓ Test 1: Object in Front of Camera PASS\n";

    // === Test 2: Object Behind Camera (Culled) ===
    std::cout << "[TEST 2/5] Running Object Behind Camera test...\n";
    Vec3 behind_obj{0.0, 0.0, 15.0};
    CHECK(!cam.is_visible(behind_obj, 1.0), "Object behind camera must be culled");
    std::cout << "  ✓ Test 2: Object Behind Camera PASS\n";

    // === Test 3: Object Beyond Far Plane (Culled) ===
    std::cout << "[TEST 3/5] Running Object Beyond Far Plane test...\n";
    Vec3 far_obj{0.0, 0.0, -2000.0};
    CHECK(!cam.is_visible(far_obj, 1.0), "Object beyond far plane must be culled");
    std::cout << "  ✓ Test 3: Object Beyond Far Plane PASS\n";

    // === Test 4: Frustum Culling Integration in Render Pipeline ===
    std::cout << "[TEST 4/5] Running Frustum Culling Integration test...\n";
    Mesh3D mesh;
    mesh.vertices = {{{0,0,0}, {0,0,1}, 0, 0}, {{1,0,0}, {0,0,1}, 1, 0}, {{0,1,0}, {0,0,1}, 0, 1}};
    mesh.indices = {0, 1, 2};

    Render3DPipeline pipeline;
    pipeline.submit_mesh(mesh, Material3D{}, {0.0, 0.0, 5.0});  // Visible
    pipeline.submit_mesh(mesh, Material3D{}, {0.0, 0.0, 25.0}); // Culled behind

    std::size_t rendered = pipeline.render_frame(cam);
    CHECK(rendered == 1, "Only 1 visible mesh triangle must be rendered");
    CHECK(pipeline.culled_meshes() == 1, "1 mesh must be culled");
    std::cout << "  ✓ Test 4: Frustum Culling Integration PASS\n";

    // === Test 5: Large Bounding Radius Visibility ===
    std::cout << "[TEST 5/5] Running Large Bounding Radius Visibility test...\n";
    Vec3 edge_obj{0.0, 0.0, 11.5}; // Slightly behind position (10.0), but radius 2.0 reaches near range
    CHECK(cam.is_visible(edge_obj, 3.0), "Large bounding radius reaching frustum must be visible");
    std::cout << "  ✓ Test 5: Large Bounding Radius Visibility PASS\n";

    std::cout << "\n========================================\n";
    std::cout << "🎉 ALL 5 FRUSTUM CULLING TESTS PASSED PERFECTLY!\n";
    std::cout << "========================================\n";
    return 0;
}
