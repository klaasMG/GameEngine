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

class RenderQueue {
public:
    void send_render_data(const RenderData& render_data, RenderId render_id);
    void send_mesh(const Mesh& mesh, RenderId render_id);
    Result<RenderDataAndId> get_render_data();
    Result<MeshAndId> get_mesh();
private:
    std::queue<RenderDataAndId> render_data_queue;
    std::queue<MeshAndId> mesh_queue;
};

class GameSimulation {
public:
    GameSimulation();
    ~GameSimulation();
    void run();
private:
    entt::registry registry;
};