#pragma comment(lib, "winmm.lib")
#include "game_simulation.h"
#include <fstream>
#include "renderer.h"
#include <mutex>
#include "vendor/glm/ext/matrix_transform.hpp"
#include <vendor/glm/glm.hpp>
#include <iostream>
#include <ranges>
#define CHUNK_SIZE 16

void printMat4(const glm::mat4& m) {
    for (int row = 0; row < 4; row++) {
        for (int col = 0; col < 4; col++) {
            std::cout << m[col][row] << " ";
        }
        std::cout << "\n";
    }
}

Mesh create_block_mesh(glm::vec3 position, float object_id, int32_t block_id){
    float block_id_float = static_cast<float>(block_id);
    std::vector<float> base_vertices = {
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 0.0f,
         0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f,
         0.5f,  0.5f, -0.5f, 1.0f, 1.0f, 0.0f, 0.0f,
         0.5f,  0.5f, -0.5f, 1.0f, 1.0f, 0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 0.0f,

        -0.5f, -0.5f,  0.5f, 0.0f, 0.0f, 0.0f, 0.0f,
         0.5f, -0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 0.0f,
         0.5f,  0.5f,  0.5f, 1.0f, 1.0f, 0.0f, 0.0f,
         0.5f,  0.5f,  0.5f, 1.0f, 1.0f, 0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 0.0f,
        -0.5f, -0.5f,  0.5f, 0.0f, 0.0f, 0.0f, 0.0f,

        -0.5f,  0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f,
        -0.5f, -0.5f,  0.5f, 1.0f, 1.0f, 0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 0.0f,

         0.5f,  0.5f,  0.5f, 1.0f, 1.0f, 0.0f, 0.0f,
         0.5f,  0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f,
         0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 0.0f,
         0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 0.0f,
         0.5f, -0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 0.0f,
         0.5f,  0.5f,  0.5f, 1.0f, 1.0f, 0.0f, 0.0f,

        -0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f,
         0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 0.0f, 0.0f,
         0.5f, -0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 0.0f,
         0.5f, -0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 0.0f,
        -0.5f, -0.5f,  0.5f, 0.0f, 0.0f, 0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f,

        -0.5f,  0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f,
         0.5f,  0.5f, -0.5f, 1.0f, 1.0f, 0.0f, 0.0f,
         0.5f,  0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 0.0f,
         0.5f,  0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f, 0.0f, 0.0f, 0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f
    };
    std::vector<float> vertices = {};
    vertices.reserve(base_vertices.size());
    for (size_t i = 0; i < base_vertices.size(); i += 7) {
        vertices.push_back(base_vertices[i] + position.x);
        vertices.push_back(base_vertices[i + 1] + position.y);
        vertices.push_back(base_vertices[i + 2] + position.z);
        vertices.push_back(base_vertices[i + 3]);
        vertices.push_back(base_vertices[i + 4]);
        vertices.push_back(object_id);
        vertices.push_back(block_id_float);
    }
    return Mesh{.mesh = vertices};
};

glm::mat4x4 create_model_matrix(const glm::vec3& position) {
    glm::mat4x4 model = glm::mat4(1.0f);
    model = glm::translate(model, position);
    return model;
}

glm::mat4x4 create_model_matrix_chunk(const glm::vec3& position) {
    glm::vec3 chunk_position = glm::vec3(position.x * CHUNK_SIZE, position.y * CHUNK_SIZE, position.z * CHUNK_SIZE);
    glm::mat4x4 model = create_model_matrix(chunk_position);
    return model;
};

GameSimulation::GameSimulation(std::shared_ptr<RenderQueue> render_queue_in) {
    render_queue = render_queue_in;
    registry = entt::registry();
    running = true;
}

std::vector<float> generate_chunk_mesh(const ChunkBlockData& block_data_chunk, const float& obj_id) {
    std::vector<float> chunk_mesh = {};
    std::array<uint32_t, 4096> block_data = block_data_chunk.block_type;
    for (int i = 0; i < block_data.size(); i++) {
        if (block_data[i] == 0) {
            continue;
        }
        glm::ivec3 pos = glm::ivec3{i % 16,(i / 16) % 16,i / 256};
        glm::ivec3 left_pos = { pos.x - 1, pos.y, pos.z };
        glm::ivec3 right_pos = { pos.x + 1, pos.y, pos.z };
        glm::ivec3 down_pos = { pos.x,pos.y - 1, pos.z };
        glm::ivec3 up_pos = { pos.x, pos.y + 1, pos.z };
        glm::ivec3 back_pos = { pos.x, pos.y, pos.z - 1 };
        glm::ivec3 forward_pos = { pos.x, pos.y,pos.z + 1 };
        int left_idx = (left_pos.x >= 0 && left_pos.x < 16) ? left_pos.x + left_pos.y * 16 + left_pos.z * 256 : -1;
        int right_idx = (right_pos.x >= 0 && right_pos.x < 16) ? right_pos.x + right_pos.y * 16 + right_pos.z * 256 : -1;
        int down_idx = (down_pos.y >= 0 && down_pos.y < 16) ? down_pos.x + down_pos.y * 16 + down_pos.z * 256 : -1;
        int up_idx = (up_pos.y >= 0 && up_pos.y < 16) ? up_pos.x + up_pos.y * 16 + up_pos.z * 256 : -1;
        int back_idx = (back_pos.z >= 0 && back_pos.z < 16) ? back_pos.x + back_pos.y * 16 + back_pos.z * 256 : -1;
        int forward_idx = (forward_pos.z >= 0 && forward_pos.z < 16) ? forward_pos.x + forward_pos.y * 16 + forward_pos.z * 256 : -1;
        bool any_is_zero = false;
        std::array<int, 6> idx_array = {forward_idx, back_idx, left_idx, right_idx, down_idx, up_idx};
        for (int idx : idx_array) {
            if (idx > block_data.size()) {
                any_is_zero = true;
            }
        }
        if (!any_is_zero) {
            if (block_data[left_idx] == 0 || block_data[right_idx] == 0 || block_data[down_idx] == 0 || block_data[up_idx] == 0 || block_data[back_idx] == 0 || block_data[forward_idx] == 0) {
                any_is_zero = true;
            }
        }
        if (any_is_zero) {
            glm::vec3 float_pos = glm::vec3(pos.x, pos.y, pos.z);
            Mesh block_mesh = create_block_mesh(float_pos, obj_id, block_data[i]);
            chunk_mesh.insert(chunk_mesh.end(), block_mesh.mesh.begin(), block_mesh.mesh.end());;
        }
    }
    return chunk_mesh;
};

void GameSimulation::run() {
    std::vector<float> vertices = {
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 0.0f,
         0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f,
         0.5f,  0.5f, -0.5f, 1.0f, 1.0f, 0.0f, 0.0f,
         0.5f,  0.5f, -0.5f, 1.0f, 1.0f, 0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 0.0f,

        -0.5f, -0.5f,  0.5f, 0.0f, 0.0f, 0.0f, 0.0f,
         0.5f, -0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 0.0f,
         0.5f,  0.5f,  0.5f, 1.0f, 1.0f, 0.0f, 0.0f,
         0.5f,  0.5f,  0.5f, 1.0f, 1.0f, 0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 0.0f,
        -0.5f, -0.5f,  0.5f, 0.0f, 0.0f, 0.0f, 0.0f,

        -0.5f,  0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f,
        -0.5f, -0.5f,  0.5f, 1.0f, 1.0f, 0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 0.0f,

         0.5f,  0.5f,  0.5f, 1.0f, 1.0f, 0.0f, 0.0f,
         0.5f,  0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f,
         0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 0.0f,
         0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 0.0f,
         0.5f, -0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 0.0f,
         0.5f,  0.5f,  0.5f, 1.0f, 1.0f, 0.0f, 0.0f,

        -0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f,
         0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 0.0f, 0.0f,
         0.5f, -0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 0.0f,
         0.5f, -0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 0.0f,
        -0.5f, -0.5f,  0.5f, 0.0f, 0.0f, 0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f,

        -0.5f,  0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f,
         0.5f,  0.5f, -0.5f, 1.0f, 1.0f, 0.0f, 0.0f,
         0.5f,  0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 0.0f,
         0.5f,  0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f, 0.0f, 0.0f, 0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f
    };
    Mesh mesh = create_block_mesh(glm::vec3{-0.0f, -0.0f, -0.0f},0.0,0.0);
    RenderData object_render_data = RenderData{.model_matrix = glm::mat4(1.0f)};
    RenderId render_id = RenderId{.render_id = 0};
    using clock = std::chrono::high_resolution_clock;
    entt::entity chunck = entt::null;
    while (running) {
        clock::time_point start = clock::now();
        clock::time_point target = start + std::chrono::milliseconds(10);
        if (chunck == entt::null) {
            chunck = registry.create();
            chunks.push_back(chunck);
            ChunkId chunk_id = ChunkId{chunks.size() - 1};
            registry.emplace<ChunkId>(chunck, chunk_id);
            if (chunk_id.chunk_id > chunks.size() - 1) {
                chunks.resize(chunk_id.chunk_id + 1);
            }
            glm::vec3 chunk_position = glm::vec3{0.0f, 1.0f, 0.0f};
            registry.emplace<ChunkPosition>(chunck, ChunkPosition{.position = chunk_position});
            std::array<uint32_t, 4096> arr;
            arr.fill(1);
            ChunkBlockData chunk_block_data = ChunkBlockData{};
            chunk_block_data.block_type = arr;
            registry.emplace<ChunkBlockData>(chunck, chunk_block_data);
            glm::mat4x4 model_matrix = create_model_matrix_chunk(chunk_position);
            std::cout << "model matrix 1: " << std::endl;
            printMat4(model_matrix);
            registry.emplace<RenderData>(chunck, RenderData{.model_matrix = model_matrix});
            registry.emplace<RenderId>(chunck, render_id);
            Mesh chunk_mesh = Mesh{.mesh = generate_chunk_mesh(chunk_block_data, render_id.render_id)};
            registry.emplace<Mesh>(chunck, chunk_mesh);
            render_queue->send_mesh(chunk_mesh, render_id);
            render_queue->send_render_data(RenderData{.model_matrix = model_matrix}, render_id);

            entt::entity chunck2 = registry.create();
            chunks.push_back(chunck2);
            ChunkId chunk_id2 = ChunkId{chunks.size() - 1};
            registry.emplace<ChunkId>(chunck2, chunk_id2);
            if (chunk_id2.chunk_id > chunks.size() - 1) {
                chunks.resize(chunk_id2.chunk_id + 1);
            }
            glm::vec3 chunk_position2 = glm::vec3{0.0f, 2.0f, 0.0f};
            registry.emplace<ChunkPosition>(chunck2, ChunkPosition{.position = chunk_position2});
            std::array<uint32_t, 4096> arr2;
            arr2.fill(1);
            ChunkBlockData chunk_block_data2 = ChunkBlockData{};
            chunk_block_data2.block_type = arr2;
            registry.emplace<ChunkBlockData>(chunck2, chunk_block_data2);
            glm::mat4x4 model_matrix2 = create_model_matrix_chunk(chunk_position2);
            std::cout << "model matrix 2: " << std::endl;
            printMat4(model_matrix2);
            registry.emplace<RenderData>(chunck2, RenderData{.model_matrix = model_matrix2});
            RenderId render_id2 = RenderId{.render_id = 1};
            registry.emplace<RenderId>(chunck2, render_id2);
            Mesh chunk_mesh2 = Mesh{.mesh = generate_chunk_mesh(chunk_block_data2, render_id2.render_id)};
            registry.emplace<Mesh>(chunck2, chunk_mesh2);
            render_queue->send_mesh(chunk_mesh2, render_id2);
            render_queue->send_render_data(RenderData{.model_matrix = model_matrix2}, render_id2);

        }
        while (clock::now() < target) {
            std::this_thread::yield();
        }
        clock::time_point end = clock::now();
        std::chrono::microseconds real = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        std::cout << "real loop: " << real.count() << " us\n";
    }

}

void GameSimulation::stop() {
    running = false;
}

event_available_type RenderQueue::is_event_available() {
    std::lock_guard<std::mutex> lock(mutex);
    if (!mesh_queue.empty()) {
        return event_available_type::MESH;
    } else if (!render_data_queue.empty()) {
        return event_available_type::RENDER_DATA;
    }
    return event_available_type::NONE;
}

void RenderQueue::wait_for_event() {
    std::unique_lock<std::mutex> lock(mutex);
    cv.wait(lock, [this] {
        return !mesh_queue.empty() || !render_data_queue.empty();
    });
}

void RenderQueue::send_render_data(const RenderData& render_data, RenderId render_id) {
    {
        std::lock_guard<std::mutex> lock(mutex);
        RenderDataAndId render_data_and_id = RenderDataAndId{.render_data = render_data, .render_id = render_id.render_id};
        render_data_queue.push(render_data_and_id);
    }
    cv.notify_one();
}

void RenderQueue::send_mesh(const Mesh& mesh, RenderId render_id) {
    {
        std::lock_guard<std::mutex> lock(mutex);
        MeshAndId mesh_and_id = MeshAndId{.mesh = mesh.mesh, .render_id = render_id.render_id};
        mesh_queue.push(mesh_and_id);
    }
    cv.notify_one();
}

Result<RenderDataAndId> RenderQueue::get_render_data() {
    std::lock_guard<std::mutex> lock(mutex);
    if (render_data_queue.empty()) {
        ErrorType error_type = ErrorType::QUEUE_EMPTY;
        return Result<RenderDataAndId>{error_type};
    }
    RenderDataAndId render_data_and_id = render_data_queue.front();
    render_data_queue.pop();
    return Result<RenderDataAndId>{render_data_and_id};
}

Result<MeshAndId> RenderQueue::get_mesh() {
    std::lock_guard<std::mutex> lock(mutex);
    if (mesh_queue.empty()) {
        ErrorType error_type = ErrorType::QUEUE_EMPTY;
        return Result<MeshAndId>{error_type};
    }
    MeshAndId mesh_and_id = mesh_queue.front();
    mesh_queue.pop();
    return Result<MeshAndId>{mesh_and_id};
}