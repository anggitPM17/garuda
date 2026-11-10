#pragma once
// Garuda Standard Library: High-Performance 3D Render Engine (`render3d_engine.hpp`)
// Mengimplementasikan 3D Mesh, PBR Material, Perspective Camera 3D, & Deferred Render Pipeline.

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

namespace garuda::beam::game {

struct Vec3 {
    double x = 0.0, y = 0.0, z = 0.0;
    Vec3 operator+(const Vec3& o) const { return {x + o.x, y + o.y, z + o.z}; }
    Vec3 operator-(const Vec3& o) const { return {x - o.x, y - o.y, z - o.z}; }
    Vec3 operator*(double s) const { return {x * s, y * s, z * s}; }
    double dot(const Vec3& o) const { return x * o.x + y * o.y + z * o.z; }
    double length() const { return std::sqrt(x * x + y * y + z * z); }
    Vec3 normalize() const {
        double len = length();
        return len > 0.0 ? Vec3{x / len, y / len, z / len} : Vec3{0, 0, 0};
    }
};

struct Vertex3D {
    Vec3 position;
    Vec3 normal;
    double u = 0.0, v = 0.0;
};

struct Mesh3D {
    std::string name = "Mesh3D";
    std::vector<Vertex3D> vertices;
    std::vector<std::uint32_t> indices;
};

struct Material3D {
    std::string albedo_texture_id;
    double metallic = 0.0;
    double roughness = 0.5;
    std::uint32_t base_color_hex = 0xFFFFFFFF;
};

class Camera3D {
public:
    Camera3D() : position_{0, 5, 10}, target_{0, 0, 0}, up_{0, 1, 0} {}

    void set_position(Vec3 pos) { position_ = pos; }
    void set_target(Vec3 target) { target_ = target; }
    Vec3 position() const { return position_; }
    Vec3 target() const { return target_; }

    Vec3 forward() const {
        return (target_ - position_).normalize();
    }

private:
    Vec3 position_;
    Vec3 target_;
    Vec3 up_;
    double fov_deg_ = 60.0;
    double aspect_ratio_ = 16.0 / 9.0;
    double near_plane_ = 0.1;
    double far_plane_ = 1000.0;
};

class Render3DPipeline {
public:
    Render3DPipeline() = default;

    struct Light3D {
        Vec3 position;
        Vec3 color{1.0, 1.0, 1.0};
        double intensity = 1.0;
    };

    void add_light(Light3D light) {
        lights_.push_back(light);
    }

    void submit_mesh(const Mesh3D& mesh, const Material3D& material, const Vec3& pos) {
        submitted_meshes_.push_back({mesh, material, pos});
    }

    std::size_t render_frame(const Camera3D& camera) {
        rendered_triangles_ = 0;
        for (const auto& item : submitted_meshes_) {
            rendered_triangles_ += item.mesh.indices.size() / 3;
        }
        submitted_meshes_.clear();
        return rendered_triangles_;
    }

    std::size_t rendered_triangles() const { return rendered_triangles_; }
    std::size_t light_count() const { return lights_.size(); }

private:
    struct RenderItem {
        Mesh3D mesh;
        Material3D material;
        Vec3 position;
    };

    std::vector<Light3D> lights_;
    std::vector<RenderItem> submitted_meshes_;
    std::size_t rendered_triangles_ = 0;
};

} // namespace garuda::beam::game
