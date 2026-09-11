#pragma once
#include <vector>
#include "vendor/glad/include/glad/glad.h"
#include "vendor/glfw/include/GLFW/glfw3.h"
#include "vendor/glm/glm.hpp"
#include "texture_loader.h"
#include "Camera.h"
#include "structs.h"

class Renderer {
public:
    Renderer(std::shared_ptr<input_queue> input_queue_ptr);
    ~Renderer();
    bool initialize();
    void run();
    void cleanup();
    void set_camera(Camera camera);
    LoadedImage* block_texture = nullptr;
    GLFWwindow* window;
    GLuint shaderProgram;
    GLuint VAO, VBO;
    GLuint cameraUBO;
    GLuint SSBO;
    GameData game_data;
    static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
    static void mouse_callback(GLFWwindow* window, double xpos, double ypos);
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
    bool vertex_check = false;
    std::shared_ptr<input_queue> input_queue_ptr;
    std::timed_mutex lock;
};