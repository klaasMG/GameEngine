#pragma once
#include <queue>
#include <mutex>
#include <condition_variable>

#include "Camera.h"
#include "lib/Error.h"
#include "entt/entt.hpp"
#include "vendor/glm/glm.hpp"
#include "vendor/glm/ext/matrix_clip_space.hpp"

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

struct InputData {
    std::string input;
    double deltaX = 0;
    double deltaY = 0;
};

struct ChunkBlockData {
    std::array<uint32_t, 4096> block_type;
};

struct ChunkPosition {
    glm::vec3 position;
};

struct ChunkId {
    size_t chunk_id;
};

enum class Direction {
    UP,
    DOWN,
    LEFT,
    RIGHT,
    FORWARD,
    BACK,
};

enum class event_available_type {
    NONE = 0,
    MESH = 1,
    RENDER_DATA = 2,
    INPUT = 3,
    CAMERA = 5,
};

class RenderQueue {
public:
    void send_render_data(const RenderData& render_data, RenderId render_id);
    void send_mesh(const Mesh& mesh, RenderId render_id);
    void send_input(const std::string& input, const double& deltaX = 0, const double& deltaY = 0);
    void send_camera(const Camera& camera);
    Result<Camera> get_camera_data();
    Result<InputData> get_input_data();
    Result<RenderDataAndId> get_render_data();
    Result<MeshAndId> get_mesh();
    event_available_type is_event_available();
    void wait_for_event();
private:
    std::queue<Camera> camera_queue;
    std::queue<InputData> input_data_queue;
    std::queue<RenderDataAndId> render_data_queue;
    std::queue<MeshAndId> mesh_queue;
    std::mutex mutex;
    std::condition_variable cv;
};

struct ChunkData {
    ChunkId chunk_id;
    ChunkBlockData chunk_block_data;
    ChunkPosition chunk_position;
    entt::entity entity;
};

class GameSimulation {
public:
    GameSimulation(std::shared_ptr<RenderQueue> render_queue_in);
    void run();
    void stop();
    void set_camera(const Camera& camera);
    std::vector<ChunkData> generate_chunk(const size_t& pos_x, const size_t& pos_y);
private:
    double lastMouseX = 0.0;
    double lastMouseY = 0.0;
    const size_t render_distance = 16;
    Camera active_camera{
        .projection_matrix = glm::perspective(glm::radians(60.0f), (float)800.0 / (float)600.0, 0.1f, 5000.0f),
        .view_matrix = glm::mat4(1.0f),
        .position = glm::vec3(0.0f, 45.0f, 3.0f),
        .yaw = 0.0f,
        .pitch = 0.0f
    };
    float moveSpeed = 0.1;
    float sensitivity = 0.1;
    std::array<int64_t, 3> position = {0, 0, 0};
    std::map<glm::vec3, entt::entity> chunks_pos;
    std::vector<entt::entity> chunks = {};
    entt::registry registry;
    std::shared_ptr<RenderQueue> render_queue;
    bool running;
};