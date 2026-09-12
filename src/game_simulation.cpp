#pragma comment(lib, "winmm.lib")
#include "game_simulation.h"
#include <fstream>
#include "structs.h"
#include <mutex>
#include <thread>
#include "vendor/glm/ext/matrix_transform.hpp"
#include <vendor/glm/glm.hpp>
#include <iostream>
#include <ranges>
#include "cmath"
#define CHUNK_SIZE 16

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

GameSimulation::GameSimulation(std::shared_ptr<Renderer> render, std::shared_ptr<input_queue> input_queue_ptr) {
    this->input_queue_ptr = std::move(input_queue_ptr);
    render_for_swap = std::move(render);
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
}

void GameSimulation::set_camera(const Camera& camera) {
    game_data.camera = camera;
}

std::vector<ChunkData> GameSimulation::generate_chunk(const int64_t& pos_x, const int64_t& pos_z) {
    std::vector<ChunkData> chunks_data = {};
    entt::entity chunk = registry.create();
    ChunkId chunk_id = static_cast<ChunkId>(this->chunk_id);
    this->chunk_id++;
    std::array<uint32_t, 4096> arr;
    arr.fill(1);
    ChunkBlockData block_data = ChunkBlockData{.block_type = arr};
    ChunkPosition position = ChunkPosition{.position = {pos_x, 0, pos_z}};
    ChunkData chunk_data = ChunkData{.chunk_id = chunk_id, .chunk_block_data = block_data, .chunk_position = position, .entity = chunk};
    chunks_data.push_back(chunk_data);
    entt::entity chunk1 = registry.create();
    ChunkId chunk_id1 = static_cast<ChunkId>(this->chunk_id);
    this->chunk_id++;
    std::array<uint32_t, 4096> arr1;
    arr1.fill(1);
    ChunkBlockData block_data1 = ChunkBlockData{.block_type = arr1};
    ChunkPosition position1 = ChunkPosition{.position = {pos_x, 1, pos_z}};
    ChunkData chunk_data1 = ChunkData{.chunk_id = chunk_id1, .chunk_block_data = block_data1, .chunk_position = position1, .entity = chunk1};
    chunks_data.push_back(chunk_data1);
    return chunks_data;
}

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
    using clock = std::chrono::high_resolution_clock;
    while (running) {
        bool is_mouse_moved = false;
        clock::time_point start = clock::now();
        clock::time_point target = start + std::chrono::milliseconds(10);
        while (!input_queue_ptr->empty()) {
            float yawRad = glm::radians(game_data.camera.yaw);
            glm::vec3 forward(cos(yawRad), 0.0f, sin(yawRad));
            glm::vec3 right(sin(yawRad), 0.0f, -cos(yawRad));
            InputData input_data = input_queue_ptr->receive_input_data();
            if (input_data.key_maybe.has_value()) {
                KeyInputData key_data = input_data.key_maybe.value();
                if (key_data.key == "d") {
                    game_data.camera.position -= right * moveSpeed;
                }
                if (key_data.key == "s") {
                    game_data.camera.position -= forward * moveSpeed;
                }
                if (key_data.key == "w") {
                    game_data.camera.position += forward * moveSpeed;
                }
                if (key_data.key == "a") {
                    game_data.camera.position += right * moveSpeed;
                }
                if (key_data.key == "space") {
                    game_data.camera.position.y += moveSpeed;
                }
                if (key_data.key == "shift") {
                    game_data.camera.position.y -= moveSpeed;
                }
            }
            else if (input_data.mouse_maybe.has_value()) {
                MouseInputData mouse_input_data = input_data.mouse_maybe.value();
                double xpos = mouse_input_data.posX;
                double ypos = mouse_input_data.posY;
                double deltaX = xpos - lastMouseX;
                double deltaY = ypos - lastMouseY;
                lastMouseX = xpos;
                lastMouseY = ypos;

                Camera camera = game_data.camera;

                camera.yaw += deltaX * sensitivity;
                camera.pitch -= deltaY * sensitivity;

                if (camera.pitch > 90.0f){
                    camera.pitch = 90.0f;
                }
                if (camera.pitch < -90.0f){
                    camera.pitch = -90.0f;
                }
                float yawRad = glm::radians(camera.yaw);
                float pitchRad = glm::radians(camera.pitch);
                glm::vec3 front;
                front.x = cos(yawRad) * cos(pitchRad);
                front.y = sin(pitchRad);
                front.z = sin(yawRad) * cos(pitchRad);
                glm::vec3 cameraTarget = camera.position + glm::normalize(front);
                camera.view_matrix = glm::lookAt(camera.position, cameraTarget, glm::vec3(0.0f, 1.0f, 0.0f));
                set_camera(camera);
                is_mouse_moved = true;
            }
            float pitchRad = glm::radians(game_data.camera.pitch);
            glm::vec3 front;
            front.x = cos(yawRad) * cos(pitchRad);
            front.y = sin(pitchRad);
            front.z = sin(yawRad) * cos(pitchRad);
            glm::vec3 cameraTarget = game_data.camera.position + glm::normalize(front);
            game_data.camera.view_matrix = glm::lookAt(game_data.camera.position, cameraTarget, glm::vec3(0.0f, 1.0f, 0.0f));
        }
        if (screen_size_last != game_data.screen_size) {
            game_data.camera.projection_matrix = glm::perspective(
                glm::radians(60.0f),
                game_data.screen_size.at(0) / game_data.screen_size.at(1),
                0.1f,
                5000.0f
            );
            screen_size_last = game_data.screen_size;
        }
        const glm::vec2 chunk_pos_player = glm::vec2{std::floor(game_data.camera.position.x / 16.0f), std::floor(game_data.camera.position.z / 16.0f)};
        std::vector<entt::entity> chunks_to_remove = {};
        for (const std::pair<const glm::vec<3, float>, entt::entity>& chunk : chunks) {
            float dx = chunk.first.x - chunk_pos_player.x;
            float dz = chunk.first.z - chunk_pos_player.y;
            float chunk_to_player_distance_sqaured = (dx * dx) + (dz * dz);
            if (chunk_to_player_distance_sqaured > render_distance * render_distance) {
                chunks_to_remove.push_back(chunk.second);
            }
        }
        std::vector<glm::vec2> missing_chunks = {};
        for (int dx = -static_cast<int>(render_distance); dx <= static_cast<int>(render_distance); ++dx) {
            for (int dz = -static_cast<int>(render_distance); dz <= static_cast<int>(render_distance); ++dz) {
                if (dx * dx + dz * dz > render_distance * render_distance) continue;
                glm::vec2 target{chunk_pos_player.x + dx, chunk_pos_player.y + dz};
                bool found = false;
                for (const std::pair<const glm::vec3, entt::entity>& chunk : chunks) {
                    const glm::vec<3, float> key = chunk.first;
                    if (key.x == target.x && target.y == key.z) {
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    missing_chunks.push_back(target);
                }
            }
        }
        for (entt::entity& to_remove : chunks_to_remove) {
            ChunkPosition chunk_position = {};
            try {
                 chunk_position = registry.get<ChunkPosition>(to_remove);
            }
            catch (const std::bad_function_call& e) {
                std::cout << e.what() << std::endl;
                throw std::runtime_error("why the fuck is this");
            }
            catch (const std::runtime_error& e) {
                std::cout << e.what() << std::endl;
                throw std::runtime_error("fuck off");
            }
            for (auto it = chunks.begin(); it != chunks.end(); ) {
                if (it->first.x == chunk_position.position.x &&
                    it->first.y == chunk_position.position.y &&
                    it->first.z == chunk_position.position.z) {
                    RenderId render_id_for_deleting = registry.get<RenderId>(to_remove);
                    free_render_ids.push(render_id_for_deleting.render_id);
                    game_data.object_vertex_data.at(render_id_for_deleting.render_id) = {};
                    registry.destroy(it->second);
                    chunks.erase(it);
                    break;
                    }
                else {
                    ++it;
                }
            }
        }
        for (const glm::vec2& missing_chunk_pos : missing_chunks) {
            std::vector<ChunkData> chunk = generate_chunk(missing_chunk_pos.x, missing_chunk_pos.y);
            for (const ChunkData& chunk_piece : chunk) {
                entt::entity entity = chunk_piece.entity;
                registry.emplace<ChunkId>(entity, chunk_piece.chunk_id);
                registry.emplace<ChunkPosition>(entity, chunk_piece.chunk_position);
                registry.emplace<ChunkBlockData>(entity, chunk_piece.chunk_block_data);
                size_t render_id_local = render_id;
                if (!free_render_ids.empty()) {
                    render_id_local = free_render_ids.front();
                    free_render_ids.pop();
                }
                else {
                    render_id++;
                }
                registry.emplace<RenderId>(entity, RenderId{render_id_local});
                std::vector<float> mesh_data = generate_chunk_mesh(chunk_piece.chunk_block_data, render_id_local);
                glm::mat4x4 model_matrix_data = create_model_matrix_chunk(chunk_piece.chunk_position.position);
                Mesh mesh = Mesh{.mesh = mesh_data};
                ObjectRenderData model_matrix = ObjectRenderData{.model_matrix = model_matrix_data};
                registry.emplace<ObjectRenderData>(entity,model_matrix);
                registry.emplace<Mesh>(entity, mesh);
                if (render_id_local >= game_data.model_matrices.size()) {
                    game_data.model_matrices.resize(render_id_local + 1);
                }
                if (render_id_local >= game_data.object_vertex_data.size()) {
                    game_data.object_vertex_data.resize(render_id_local + 1);
                }
                game_data.model_matrices.at(render_id_local) = model_matrix;
                game_data.object_vertex_data.at(render_id_local) = mesh_data;
                chunks.emplace(chunk_piece.chunk_position.position, entity);
            }
        }
        if (is_mouse_moved) {
            glm::vec2 mouse_pos = {game_data.screen_size.at(0) / 2.0, game_data.screen_size.at(1) / 2.0};
            float x_ndc = (2.0 * mouse_pos.x) / game_data.screen_size.at(0) - 1;
            float y_ndc = 1.0 - ((2.0 * mouse_pos.y) / game_data.screen_size.at(1));
            glm::vec4 P_ndc = glm::vec4{x_ndc, y_ndc, -1.0f, -1.0f};
            glm::vec4 P_world = glm::inverse(game_data.camera.projection_matrix * game_data.camera.view_matrix) * P_ndc;
            P_world = P_world / P_world.w;
            glm::vec4 ray_origin = glm::vec4{game_data.camera.position, 1.0};
            glm::vec4 ray_direction = glm::normalize(P_world - ray_origin);
            camera_ray_origin = ray_origin;
            camera_ray_direction = ray_direction;


            is_mouse_moved = false;
        }
        std::chrono::milliseconds time_for_lock = std::chrono::duration_cast<std::chrono::milliseconds>(target - clock::now());
        swap_data(time_for_lock);
        while (clock::now() < target) {
            std::this_thread::yield();
        }
    }

}

void GameSimulation::stop() {
    running = false;
}


void GameSimulation::swap_data(const std::chrono::milliseconds& time) {
    if (render_for_swap->lock.try_lock_for(time)) {
        render_for_swap->game_data = this->game_data;
        std::swap(render_for_swap->game_data, this->game_data);
        render_for_swap->lock.unlock();
    }
}
