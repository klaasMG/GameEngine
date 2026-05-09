#include "Renderer.h"

int main() {
    Renderer renderer;
    if (!renderer.initialize()) {
        return -1;
    }
    renderer.run();
    return 0;
}