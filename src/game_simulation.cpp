#include "game_simulation.h"

GameSimulation::GameSimulation() {
    registry = entt::registry();
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
