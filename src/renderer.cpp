#include "renderer.h"

#include <fstream>

#include "vendor/glm/glm.hpp"
#include "vendor/glm/gtc/matrix_transform.hpp"
#include <iostream>
#include <regex>
#include <vector>

std::vector<float> flatten_vector(const std::vector<std::vector<float>>& input) {
    std::vector<float> output;

    size_t total_size = 0;
    for (const std::vector<float>& row : input) {
        total_size += row.size();
    }

    output.reserve(total_size);

    for (const std::vector<float>& row : input) {
        output.insert(output.end(), row.begin(), row.end());
    }

    return output;
}

const char* vertexShaderSource = R"(
    #version 430 core
    layout (location = 0) in vec3 aPos;
    layout (location = 1) in vec2 aUV;
    layout (location = 2) in float aObjId;
    layout (location = 3) in float aBlockId;

    out vec2 vUV;
    out float vObjId;
    out float vBlockId;
    out vec3 vPos;

    layout (std140) uniform CameraData {
        mat4 projection_matrix;
        mat4 view_matrix;
    };

    layout(std430, binding = 0) buffer ObjectMatrices {
        mat4 model_matrices[];
    };

    void main() {
        vPos = aPos;
        vUV = aUV;
        vObjId = aObjId;
        vBlockId = aBlockId;
        int ivObjId = int(vObjId);
        mat4 model_matrix = model_matrices[ivObjId];
        gl_Position = projection_matrix * view_matrix * model_matrix * vec4(aPos, 1.0);
    }
)";

const char* fragmentShaderSource = R"(
    #version 430 core
    in vec3 vPos;
    in vec2 vUV;
    in float vObjId;
    in float vBlockId;
    out vec4 FragColor;
    uniform sampler2D uBlockTextureAtlas;

    void main() {
        float tileX = mod(vBlockId - 1.0, 16.0);
        float tileY = floor((vBlockId - 1.0) / 16.0);
        vec2 atlasUV = vUV + vec2(tileX, tileY) / 16.0;
        vec4 texColor = texture(uBlockTextureAtlas, atlasUV);
        FragColor = texColor;
    }
)";

Renderer::Renderer(std::shared_ptr<input_queue> input_queue_ptr) {
    this->input_queue_ptr = std::move(input_queue_ptr);
    window = nullptr;
    shaderProgram = 0;
    VAO = 0;
    VBO = 0;
    cameraUBO = 0;
    SSBO = 0;
}

Renderer::~Renderer(){
    if (block_texture){
        block_texture->destroy();
        delete block_texture;
        block_texture = nullptr;
    }
}

void Renderer::setup_render_data_ssbo() {
    glGenBuffers(1, &SSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBO);

    lock.lock();
    // Allocate storage (example: 100 structs)
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(ObjectRenderData) * game_data.model_matrices.size(), nullptr, GL_DYNAMIC_DRAW);

    // Bind to binding point 0
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, SSBO);

    // Optional: upload initial data
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(ObjectRenderData) * game_data.model_matrices.size(), game_data.model_matrices.data());
    lock.unlock();
    // Cleanup bind
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void Renderer::update_render_data_ssbo() {
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBO);
    lock.lock();
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(ObjectRenderData) * game_data.model_matrices.size(), game_data.model_matrices.data(), GL_DYNAMIC_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    lock.unlock();
}

void Renderer::destroy_render_data_ssbo() {
    glDeleteBuffers(1, &SSBO);
    SSBO = 0;
}

void Renderer::framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
    Renderer* renderer = static_cast<Renderer*>(glfwGetWindowUserPointer(window));
    renderer->lock.lock();
    renderer->game_data.screen_size = {static_cast<float>(width), static_cast<float>(height)};
    renderer->lock.unlock();
}

void Renderer::mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    Renderer* renderer = static_cast<Renderer*>(glfwGetWindowUserPointer(window));
    MouseInputData mouse_input_data = MouseInputData{.posX = xpos, .posY = ypos};
    renderer->input_queue_ptr->send_mouse(mouse_input_data);
}

bool Renderer::initGLFW(){
    if (!glfwInit()){
        std::cerr << "Failed to initialize GLFW\n";
        return false;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(800, 600, "Triangle Renderer", nullptr, nullptr);
    glfwSetWindowUserPointer(window, this);
    if (!window){
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){
        std::cerr << "Failed to initialize GLAD\n";
        return false;
    }

    glViewport(0, 0, 800, 600);
    glEnable(GL_DEPTH_TEST);
    return true;
}

GLuint Renderer::compileShader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    int success;
    char infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

    if (!success){
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cerr << "Shader compilation failed:\n" << infoLog << "\n";
    }

    return shader;
}

void Renderer::setup_shaders(){
    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    int success;
    char infoLog[512];
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);

    if (!success){
        glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
        std::cerr << "Shader linking failed:\n" << infoLog << "\n";
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

void Renderer::setup_triangle(std::vector<float> vertices){
    if (VAO == 0){
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
    }
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(3 * sizeof(float)));
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(5 * sizeof(float)));
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);

    glBindVertexArray(0);
}

// optional: only if you want to animate vertices
void Renderer::update_triangle(std::vector<float> vertices) {
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_DYNAMIC_DRAW);
}

void Renderer::set_camera(Camera camera) {
    lock.lock();
    game_data.camera = camera;
    lock.unlock();
}

void Renderer::create_camera_ubo() {
    glGenBuffers(1, &cameraUBO);
    glBindBuffer(GL_UNIFORM_BUFFER, cameraUBO);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(Camera), nullptr, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, cameraUBO);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    GLuint blockIndex = glGetUniformBlockIndex(shaderProgram, "CameraData");
    glUniformBlockBinding(shaderProgram, blockIndex, 0);
}

void Renderer::update_camera_ubo() {
    glBindBuffer(GL_UNIFORM_BUFFER, cameraUBO);
    lock.lock();
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(Camera), &game_data.camera);
    lock.unlock();
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void Renderer::destroy_camera_ubo() {
    glDeleteBuffers(1, &cameraUBO);
}

bool Renderer::initialize(){
    if (!initGLFW()){
        return false;
    }

    block_texture = new LoadedImage{"assets/textures/dirt.png"};

    int width, height;
    glfwGetWindowSize(window, &width, &height);

    Camera camera = {
        .projection_matrix = glm::perspective(glm::radians(60.0f), (float)width / (float)height, 0.1f, 5000.0f),
        .view_matrix = glm::mat4(1.0f),
        .position = glm::vec3(0.0f, 45.0f, 3.0f),
        .yaw = 0.0f,
        .pitch = 0.0f
    };
    set_camera(camera);

    float yawRad = glm::radians(camera.yaw);
    float pitchRad = glm::radians(camera.pitch);
    glm::vec3 front;
    front.x = cos(yawRad) * cos(pitchRad);
    front.y = sin(pitchRad);
    front.z = sin(yawRad) * cos(pitchRad);
    camera.view_matrix = glm::lookAt(camera.position, camera.position + glm::normalize(front), glm::vec3(0.0f, 1.0f, 0.0f));
    set_camera(camera);

    setup_shaders();
    create_camera_ubo();
    setup_render_data_ssbo();
    std::vector<float> vertices = {
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f,
         0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f,
         0.5f,  0.5f, -0.5f, 1.0f, 1.0f, 0.0f, 1.0f,
         0.5f,  0.5f, -0.5f, 1.0f, 1.0f, 0.0f, 1.0f,
        -0.5f,  0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f,

        -0.5f, -0.5f,  0.5f, 0.0f, 0.0f, 0.0f, 1.0f,
         0.5f, -0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 1.0f,
         0.5f,  0.5f,  0.5f, 1.0f, 1.0f, 0.0f, 1.0f,
         0.5f,  0.5f,  0.5f, 1.0f, 1.0f, 0.0f, 1.0f,
        -0.5f,  0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f, 0.0f, 0.0f, 0.0f, 1.0f,

        -0.5f,  0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 1.0f,
        -0.5f,  0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f, 1.0f, 1.0f, 0.0f, 1.0f,
        -0.5f,  0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 1.0f,

         0.5f,  0.5f,  0.5f, 1.0f, 1.0f, 0.0f, 1.0f,
         0.5f,  0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f,
         0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f,
         0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f,
         0.5f, -0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 1.0f,
         0.5f,  0.5f,  0.5f, 1.0f, 1.0f, 0.0f, 1.0f,

        -0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f,
         0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 0.0f, 1.0f,
         0.5f, -0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 1.0f,
         0.5f, -0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f, 0.0f, 0.0f, 0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f,

        -0.5f,  0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f,
         0.5f,  0.5f, -0.5f, 1.0f, 1.0f, 0.0f, 1.0f,
         0.5f,  0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 1.0f,
         0.5f,  0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 1.0f,
        -0.5f,  0.5f,  0.5f, 0.0f, 0.0f, 0.0f, 1.0f,
        -0.5f,  0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f
    };
    setup_triangle(vertices);
    return true;
}

void Renderer::run() {
    while (!glfwWindowShouldClose(window)) {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS){
            glfwSetWindowShouldClose(window, true);
        }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS){
            input_queue_ptr->send_key(KeyInputData{.key = "s"});
        }
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS){
            input_queue_ptr->send_key(KeyInputData{.key = "w"});
        }
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS){
            input_queue_ptr->send_key(KeyInputData{.key = "a"});
        }
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS){
            input_queue_ptr->send_key(KeyInputData{.key = "d"});
        }
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS){
            input_queue_ptr->send_key(KeyInputData{.key = "space"});
        }
        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS){
            input_queue_ptr->send_key(KeyInputData{.key = "shift"});
        }
        lock.lock();
        std::vector<float> vertices_in = flatten_vector(game_data.object_vertex_data);
        lock.unlock();
        update_triangle(vertices_in);
        update_camera_ubo();
        update_render_data_ssbo();

        glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shaderProgram);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, block_texture->texture);
        glUniform1i(glGetUniformLocation(shaderProgram, "uBlockTextureAtlas"), 0);
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, vertices_in.size() / 7);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}

void Renderer::cleanup(){
    if (block_texture){
        block_texture->destroy();
        delete block_texture;
        block_texture = nullptr;
    }
    destroy_render_data_ssbo();
    destroy_camera_ubo();
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);

    glfwDestroyWindow(window);
    glfwTerminate();
}