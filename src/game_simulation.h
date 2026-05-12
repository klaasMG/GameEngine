#pragma once
#include <queue>
#include "lib/Error.h"
#include "entt/entt.hpp"
#include "vendor/glm/glm.hpp"

struct Mesh {
    std::vector<float> mesh;
};

struct RenderData {
    glm::mat4 model_matrix;
};

struct RenderId {
    size_t render_id;
};

struct RenderDataAndId {
    RenderData render_data;
    size_t render_id;
};

struct MeshAndId {
    Mesh mesh;
    size_t render_id;
};

enum class event_available_type {
    NONE = 0,
    MESH = 1,
    RENDER_DATA = 2,
};

class RenderQueue {
public:
    void send_render_data(const RenderData& render_data, RenderId render_id);
    void send_mesh(const Mesh& mesh, RenderId render_id);
    Result<RenderDataAndId> get_render_data();
    Result<MeshAndId> get_mesh();
    event_available_type is_event_available();
private:
    std::queue<RenderDataAndId> render_data_queue;
    std::queue<MeshAndId> mesh_queue;
};

class GameSimulation {
public:
    GameSimulation(std::shared_ptr<RenderQueue> render_queue_in);
    void run();
    void stop();
private:
    entt::registry registry;
    std::shared_ptr<RenderQueue> render_queue;
    bool running;
};