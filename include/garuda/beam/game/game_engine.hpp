#pragma once
// Garuda Standard Library: Game Engine Framework & ECS Core (`game_engine.hpp`)
// Mengimplementasikan Entity-Component-System (ECS), Physics 2D AABB Engine, Input Manager, & Render Pipeline.

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <typeindex>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace garuda::beam::game {

using Entity = std::uint64_t;

struct TransformComponent {
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;
    double rotation = 0.0;
    double scale_x = 1.0;
    double scale_y = 1.0;
};

struct RigidBodyComponent {
    double vx = 0.0;
    double vy = 0.0;
    double mass = 1.0;
    double gravity_scale = 1.0;
    bool is_static = false;
};

struct ColliderComponent {
    double width = 32.0;
    double height = 32.0;
    bool is_trigger = false;
    bool is_colliding = false;
};

struct RenderSpriteComponent {
    std::string sprite_id;
    std::uint32_t color_hex = 0xFFFFFFFF;
    int layer = 0;
};

class EcsRegistry {
public:
    EcsRegistry() = default;

    Entity create_entity() {
        Entity id = ++next_entity_id_;
        entities_.insert(id);
        return id;
    }

    void destroy_entity(Entity entity) {
        entities_.erase(entity);
        for (auto& [type, pool] : component_pools_) {
            pool->remove(entity);
        }
    }

    bool is_alive(Entity entity) const {
        return entities_.find(entity) != entities_.end();
    }

    std::size_t entity_count() const {
        return entities_.size();
    }

    template <typename T>
    void add_component(Entity entity, T component) {
        get_pool<T>()->put(entity, std::move(component));
    }

    template <typename T>
    T* get_component(Entity entity) {
        return get_pool<T>()->get(entity);
    }

    template <typename T>
    bool has_component(Entity entity) const {
        auto* pool = get_pool_const<T>();
        return pool && pool->has(entity);
    }

    template <typename T>
    std::vector<Entity> view() const {
        std::vector<Entity> result;
        auto* pool = get_pool_const<T>();
        if (!pool) return result;
        for (Entity e : entities_) {
            if (pool->has(e)) result.push_back(e);
        }
        return result;
    }

private:
    struct IComponentPool {
        virtual ~IComponentPool() = default;
        virtual void remove(Entity entity) = 0;
    };

    template <typename T>
    class ComponentPool : public IComponentPool {
    public:
        void put(Entity entity, T comp) {
            data_[entity] = std::move(comp);
        }
        T* get(Entity entity) {
            auto it = data_.find(entity);
            return it != data_.end() ? &it->second : nullptr;
        }
        bool has(Entity entity) const {
            return data_.find(entity) != data_.end();
        }
        void remove(Entity entity) override {
            data_.erase(entity);
        }
    private:
        std::unordered_map<Entity, T> data_;
    };

    template <typename T>
    ComponentPool<T>* get_pool() {
        std::type_index tid(typeid(T));
        if (component_pools_.find(tid) == component_pools_.end()) {
            component_pools_[tid] = std::make_unique<ComponentPool<T>>();
        }
        return static_cast<ComponentPool<T>*>(component_pools_[tid].get());
    }

    template <typename T>
    const ComponentPool<T>* get_pool_const() const {
        std::type_index tid(typeid(T));
        auto it = component_pools_.find(tid);
        return it != component_pools_.end() ? static_cast<const ComponentPool<T>*>(it->second.get()) : nullptr;
    }

    Entity next_entity_id_ = 0;
    std::unordered_set<Entity> entities_;
    std::unordered_map<std::type_index, std::unique_ptr<IComponentPool>> component_pools_;
};

class PhysicsEngine2D {
public:
    PhysicsEngine2D() = default;

    void update(EcsRegistry& registry, double dt) {
        double gravity = 9.8 * 10.0; // 98 px/s^2

        // 1. Velocity & Position Integration
        for (Entity e : registry.view<RigidBodyComponent>()) {
            auto* rb = registry.get_component<RigidBodyComponent>(e);
            auto* tf = registry.get_component<TransformComponent>(e);
            if (!rb || !tf || rb->is_static) continue;

            rb->vy += gravity * rb->gravity_scale * dt;
            tf->x += rb->vx * dt;
            tf->y += rb->vy * dt;
        }

        // 2. AABB Collision Detection
        auto colliders = registry.view<ColliderComponent>();
        for (std::size_t i = 0; i < colliders.size(); ++i) {
            Entity e1 = colliders[i];
            auto* c1 = registry.get_component<ColliderComponent>(e1);
            auto* t1 = registry.get_component<TransformComponent>(e1);
            if (!c1 || !t1) continue;

            for (std::size_t j = i + 1; j < colliders.size(); ++j) {
                Entity e2 = colliders[j];
                auto* c2 = registry.get_component<ColliderComponent>(e2);
                auto* t2 = registry.get_component<TransformComponent>(e2);
                if (!c2 || !t2) continue;

                bool hit = check_aabb_collision(*t1, *c1, *t2, *c2);
                if (hit) {
                    c1->is_colliding = true;
                    c2->is_colliding = true;
                }
            }
        }
    }

    static bool check_aabb_collision(
        const TransformComponent& t1, const ColliderComponent& c1,
        const TransformComponent& t2, const ColliderComponent& c2
    ) {
        return (t1.x < t2.x + c2.width &&
                t1.x + c1.width > t2.x &&
                t1.y < t2.y + c2.height &&
                t1.y + c1.height > t2.y);
    }
};

class InputEngine {
public:
    InputEngine() = default;

    void press_key(const std::string& key) {
        pressed_keys_.insert(key);
    }

    void release_key(const std::string& key) {
        pressed_keys_.erase(key);
    }

    bool is_key_pressed(const std::string& key) const {
        return pressed_keys_.find(key) != pressed_keys_.end();
    }

    void set_mouse_pos(double x, double y) {
        mouse_x_ = x;
        mouse_y_ = y;
    }

    double mouse_x() const { return mouse_x_; }
    double mouse_y() const { return mouse_y_; }

private:
    std::unordered_set<std::string> pressed_keys_;
    double mouse_x_ = 0.0;
    double mouse_y_ = 0.0;
};

class RenderPipeline {
public:
    RenderPipeline() = default;

    struct DrawCall {
        std::string sprite_id;
        double x, y;
        double scale_x, scale_y;
        std::uint32_t color;
    };

    void record_frame(const EcsRegistry& registry) {
        draw_calls_.clear();
        for (Entity e : registry.view<RenderSpriteComponent>()) {
            auto* sprite = const_cast<EcsRegistry&>(registry).get_component<RenderSpriteComponent>(e);
            auto* tf = const_cast<EcsRegistry&>(registry).get_component<TransformComponent>(e);
            if (!sprite || !tf) continue;

            draw_calls_.push_back({sprite->sprite_id, tf->x, tf->y, tf->scale_x, tf->scale_y, sprite->color_hex});
        }
    }

    std::size_t draw_call_count() const {
        return draw_calls_.size();
    }

    const std::vector<DrawCall>& draw_calls() const {
        return draw_calls_;
    }

private:
    std::vector<DrawCall> draw_calls_;
};

} // namespace garuda::beam::game
