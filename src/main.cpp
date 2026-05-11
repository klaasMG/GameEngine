#include "Renderer.h"
#include "game_simulation.h"

int main() {
    std::shared_ptr<RenderQueue> render_queue_ptr = std::make_shared<RenderQueue>();
    Renderer renderer(render_queue_ptr);
    if (!renderer.initialize()) {
        return -1;
    }
    renderer.run();
    return 0;
}