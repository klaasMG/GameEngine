#include "game_simulation.h"

#include "renderer.h"

GameSimulation::GameSimulation(std::shared_ptr<RenderQueue> render_queue_in) {
    render_queue = std::move(render_queue_in);
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
    while (running) {
        if (!is_first_data_send) {
            Mesh mesh = Mesh{.mesh = vertices};
            RenderId render_id = RenderId{.render_id = 0};
            //render_queue->send_mesh(mesh, render_id);
            is_first_data_send = true;
        }
    }
}

void GameSimulation::stop() {
    running = false;
}

event_available_type RenderQueue::is_event_available() {
    if (!mesh_queue.empty()) {
        return event_available_type::MESH;
    } else if (!render_data_queue.empty()) {
        return event_available_type::RENDER_DATA;
    }
    return event_available_type::NONE;
}

void RenderQueue::send_render_data(const RenderData& render_data, RenderId render_id) {
    RenderDataAndId render_data_and_id = RenderDataAndId{.render_data = render_data, .render_id = render_id.render_id};
    render_data_queue.push(render_data_and_id);
}

void RenderQueue::send_mesh(const Mesh& mesh, RenderId render_id) {
    MeshAndId mesh_and_id = MeshAndId{.mesh = mesh.mesh, .render_id = render_id.render_id};
    mesh_queue.push(mesh_and_id);
}

Result<RenderDataAndId> RenderQueue::get_render_data() {
    if (render_data_queue.empty()) {
        ErrorType error_type = ErrorType::QUEUE_EMPTY;
        return Result<RenderDataAndId>{error_type};
    }
    RenderDataAndId render_data_and_id = render_data_queue.front();
    render_data_queue.pop();
    return Result<RenderDataAndId>{render_data_and_id};
}

Result<MeshAndId> RenderQueue::get_mesh() {
    if (mesh_queue.empty()) {
        ErrorType error_type = ErrorType::QUEUE_EMPTY;
        return Result<MeshAndId>{error_type};
    }
    MeshAndId mesh_and_id = mesh_queue.front();
    mesh_queue.pop();
    return Result<MeshAndId>{mesh_and_id};
}
