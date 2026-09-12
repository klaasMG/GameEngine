#pragma once
#include "structs.h"
#include "Camera.h"
#include "entt/entt.hpp"
#include "vendor/glm/glm.hpp"
#include "vendor/glm/ext/matrix_clip_space.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include "renderer.h"
#include "vendor/glm/gtx/hash.hpp"

struct Mesh {
    std::vector<float> mesh;
};

struct RenderId {
    size_t render_id;
};

struct RenderDataAndId {
    ObjectRenderData render_data;
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

struct ChunkData {
    ChunkId chunk_id;
    ChunkBlockData chunk_block_data;
    ChunkPosition chunk_position;
    entt::entity entity;
};

class GameSimulation {
public:
    GameSimulation(std::shared_ptr<Renderer> render, std::shared_ptr<input_queue> input_queue_ptr);
    void swap_data(const std::chrono::milliseconds& time_for_wait);
    void run();
    void stop();
    void set_camera(const Camera& camera);
    std::vector<ChunkData> generate_chunk(const int64_t& pos_x, const int64_t& pos_z);
private:
    double lastMouseX = 0.0;
    double lastMouseY = 0.0;
    const float render_distance = 4.0;
    GameData game_data;
    glm::vec4 camera_ray_origin;
    glm::vec4 camera_ray_direction;
    size_t chunk_id = 0;
    float moveSpeed = 0.1;
    float sensitivity = 0.1;
    std::array<int64_t, 3> position = {0, 0, 0};
    std::unordered_map<glm::vec3, entt::entity> chunks = {};
    size_t render_id = 0;
    std::queue<size_t> free_render_ids = {};
    std::array<float, 2> screen_size_last = {0, 0};
    entt::registry registry;
    std::shared_ptr<Renderer> render_for_swap;
    std::shared_ptr<input_queue> input_queue_ptr;
    bool running;
};