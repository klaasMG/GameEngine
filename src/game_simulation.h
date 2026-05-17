#pragma once
#include <queue>
#include <mutex>
#include <condition_variable>
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

struct ChunkBlockData {
    std::array<uint32_t, 4096> block_type;
};

struct ChunkPosition {
    glm::vec3 position;
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
    void wait_for_event();
private:
    std::queue<RenderDataAndId> render_data_queue;
    std::queue<MeshAndId> mesh_queue;
    std::mutex mutex;
    std::condition_variable cv;
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