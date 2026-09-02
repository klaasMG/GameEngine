#pragma once
#include "vendor/glm/mat4x4.hpp"
#include "vendor/glm/vec3.hpp"

struct Camera {
    glm::mat4x4 projection_matrix;
    glm::mat4x4 view_matrix;
    glm::vec3 position = glm::vec3(0.0f, 0.0f, -3.0f);
    float yaw = 0.0f;
    float pitch = 0.0f;
};
