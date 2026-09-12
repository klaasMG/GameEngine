#pragma once
#include <string>
#include <queue>
#include <mutex>
#include <optional>
#include <utility>

#include "Camera.h"
#include "vendor/glm/ext/matrix_clip_space.hpp"

struct ObjectRenderData {
    glm::mat4x4 model_matrix;
};

struct GameData {
    std::vector<ObjectRenderData> model_matrices = {ObjectRenderData{.model_matrix = glm::mat4(1.0f)}};
    std::vector<std::vector<float>> object_vertex_data = {{}};
    std::array<float, 2> screen_size = {800.0, 600.0};
    Camera camera = Camera{
        .projection_matrix = glm::perspective(glm::radians(60.0f), (float)800.0 / (float)600.0, 0.1f, 5000.0f),
        .view_matrix = glm::mat4(1.0f),
        .position = glm::vec3(0.0f, 0.0f, 35.0f),
        .yaw = -90.0f,
        .pitch = 0.0f
    };
};

struct KeyInputData {
    std::string key;
};

struct MouseInputData {
    double posX;
    double posY;
};

struct InputData {
    std::optional<KeyInputData> key_maybe;
    std::optional<MouseInputData> mouse_maybe;
};

class input_queue {
public:
    void send_key(KeyInputData data) {
        std::lock_guard<std::mutex> lock(mtx);
        InputData event;
        event.key_maybe = std::move(data);
        queue.push(std::move(event));
    }

    void send_mouse(MouseInputData data) {
        std::lock_guard<std::mutex> lock(mtx);
        InputData event;
        event.mouse_maybe = std::move(data);
        queue.push(std::move(event));
    }

    InputData receive_input_data() {
        std::lock_guard<std::mutex> lock(mtx);
        InputData data = std::move(queue.front());
        queue.pop();
        return data;
    }

    bool empty() {
        std::lock_guard<std::mutex> lock(mtx);
        return queue.empty();
    }
    std::queue<InputData> queue;
    std::mutex mtx;
};
