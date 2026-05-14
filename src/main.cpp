#include "Renderer.h"
#include "game_simulation.h"
#include <thread>
#include "texture_loader.h"

int main() {
    std::shared_ptr<RenderQueue> render_queue_ptr = std::make_shared<RenderQueue>();
    Renderer renderer(render_queue_ptr);
    GameSimulation game_simulation = GameSimulation(render_queue_ptr);
    if (!renderer.initialize()) {
        return -1;
    }
    std::thread game_sim_thread(&GameSimulation::run, &game_simulation);
    renderer.run();
    game_simulation.stop();
    if (game_sim_thread.joinable()) {
        game_sim_thread.join();
    }
    return 0;
}