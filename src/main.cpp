#include <thread>
#include "Renderer.h"
#include "game_simulation.h"
#include "texture_loader.h"

int main() {
    std::shared_ptr<input_queue> input_queue_ptr = std::make_shared<input_queue>();
    std::shared_ptr<Renderer> renderer_ptr = std::make_shared<Renderer>(input_queue_ptr);
    GameSimulation game_simulation = GameSimulation{renderer_ptr, input_queue_ptr};
    if (!renderer_ptr->initialize()) {
        return -1;
    }
    std::thread game_sim_thread(&GameSimulation::run, &game_simulation);
    renderer_ptr->run();
    game_simulation.stop();
    if (game_sim_thread.joinable()) {
        game_sim_thread.join();
    }
    return 0;
}