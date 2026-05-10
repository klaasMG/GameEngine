#pragma once
#include <vector>
#include "vendor/glad/include/glad/glad.h"
#include "vendor/glfw/include/GLFW/glfw3.h"
#include "vendor/glm/glm.hpp"

struct Camera {
    glm::mat4x4 projection_matrix;
    glm::mat4x4 view_matrix;
};

struct ObjectRenderData {
    glm::mat4x4 model_matrix;
};

class Renderer {
public:
    Renderer();
    ~Renderer();
    bool initialize();
    void run();
    void cleanup();
    void set_camera(Camera camera);

private:
    GLFWwindow* window;
    GLuint shaderProgram;
    GLuint VAO, VBO;
    GLuint cameraUBO;
    GLuint SSBO;
    Camera active_camera;
    std::vector<ObjectRenderData> objects_render_data = {ObjectRenderData{.model_matrix = glm::mat4(1.0f)}};
    static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
    bool initGLFW();
    GLuint compileShader(GLenum type, const char* source);
    void setup_shaders();
    void setup_triangle(std::vector<float> vertices);
    void update_triangle(std::vector<float> vertices);
    void create_camera_ubo();
    void update_camera_ubo();
    void destroy_camera_ubo();
    void setup_render_data_ssbo();
    void update_render_data_ssbo();
    void destroy_render_data_ssbo();
};