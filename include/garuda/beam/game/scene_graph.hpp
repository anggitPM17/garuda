#pragma once
// Garuda Standard Library: Hierarchical 3D Scene Graph Engine (`scene_graph.hpp`)
// Mengimplementasikan SceneNode 3D Hierarchy, Spatial World Transform, Component Attachments, & Scene Graph Traversal.

#include "physics3d_engine.hpp"
#include "render3d_engine.hpp"

#include <algorithm>
#include <functional>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace garuda::beam::game {

struct Transform3D {
    Vec3 position{0.0, 0.0, 0.0};
    Vec3 rotation{0.0, 0.0, 0.0};
    Vec3 scale{1.0, 1.0, 1.0};
};

class SceneNode {
public:
    explicit SceneNode(std::string name = "SceneNode") : name_(std::move(name)) {}

    SceneNode* add_child(std::unique_ptr<SceneNode> child) {
        child->parent_ = this;
        children_.push_back(std::move(child));
        return children_.back().get();
    }

    void set_local_position(Vec3 pos) { local_transform_.position = pos; }
    void set_local_scale(Vec3 scale) { local_transform_.scale = scale; }

    Transform3D local_transform() const { return local_transform_; }
    Transform3D world_transform() const { return world_transform_; }
    std::string name() const { return name_; }

    void update_world_transform(Transform3D parent_world = Transform3D{}) {
        world_transform_.position = parent_world.position + local_transform_.position;
        world_transform_.rotation = parent_world.rotation + local_transform_.rotation;
        world_transform_.scale = {
            parent_world.scale.x * local_transform_.scale.x,
            parent_world.scale.y * local_transform_.scale.y,
            parent_world.scale.z * local_transform_.scale.z
        };

        for (auto& child : children_) {
            child->update_world_transform(world_transform_);
        }
    }

    void traverse(const std::function<void(SceneNode&)>& visitor) {
        visitor(*this);
        for (auto& child : children_) {
            child->traverse(visitor);
        }
    }

    // Attachments
    std::optional<Mesh3D> mesh;
    std::optional<Material3D> material;
    std::optional<RigidBody3D> rigid_body;
    std::optional<Collider3D> collider;

    std::size_t child_count() const { return children_.size(); }

private:
    std::string name_;
    SceneNode* parent_ = nullptr;
    std::vector<std::unique_ptr<SceneNode>> children_;
    Transform3D local_transform_;
    Transform3D world_transform_;
};

class Scene {
public:
    Scene() : root_(std::make_unique<SceneNode>("RootNode")) {}

    SceneNode* root() { return root_.get(); }
    Camera3D& active_camera() { return camera_; }
    const Camera3D& active_camera() const { return camera_; }

    void add_light(Render3DPipeline::Light3D light) {
        lights_.push_back(light);
    }

    void update(double dt) {
        root_->update_world_transform();
    }

    std::size_t render(Render3DPipeline& pipeline) {
        for (const auto& light : lights_) {
            pipeline.add_light(light);
        }

        root_->traverse([&pipeline](SceneNode& node) {
            if (node.mesh.has_value()) {
                Material3D mat = node.material.value_or(Material3D{});
                pipeline.submit_mesh(node.mesh.value(), mat, node.world_transform().position);
            }
        });

        return pipeline.render_frame(camera_);
    }

private:
    std::unique_ptr<SceneNode> root_;
    Camera3D camera_;
    std::vector<Render3DPipeline::Light3D> lights_;
};

} // namespace garuda::beam::game
