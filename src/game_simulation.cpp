#include <windows.h>
#pragma comment(lib, "winmm.lib")
#include "game_simulation.h"
#include "renderer.h"
#include <mutex>

GameSimulation::GameSimulation(std::shared_ptr<RenderQueue> render_queue_in) {
    render_queue = render_queue_in;
    registry = entt::registry();
    running = true;
}

void GameSimulation::run() {
    std::vector<float> vertices = {
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 0.0f,

        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 0.0f,

        -0.5f,  0.5f,  0.5f,  0.0f, 1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  1.0f, 1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 1.0f, 0.0f,

         0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, 1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 0.0f,

        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f
    };
    ObjectRenderData object_render_data = ObjectRenderData{.model_matrix = glm::mat4(1.0f)};
    bool is_first_data_send = false;
    using clock = std::chrono::high_resolution_clock;
    while (running) {
        clock::time_point start = clock::now();
        clock::time_point target = start + std::chrono::milliseconds(10);
        if (!is_first_data_send) {
            Mesh mesh{.mesh = vertices};
            RenderId render_id{.render_id = 0};
            render_queue->send_mesh(mesh, render_id);
            RenderData render_data{
                .model_matrix = object_render_data.model_matrix
            };
            render_queue->send_render_data(render_data, render_id);
            is_first_data_send = true;
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
