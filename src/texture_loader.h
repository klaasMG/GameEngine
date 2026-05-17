#pragma once
#include <filesystem>
#include <vector>
#include <stdexcept>
#include "vendor/stb_image/stb_image.h"
#include "vendor/glad/include/glad/glad.h"
#include "vendor/glfw/include/GLFW/glfw3.h"

namespace fs = std::filesystem;

class LoadedImage {
public:
    LoadedImage(const std::string& path, GLuint slot = 0);
    void resend();
    void destroy();
    void set_texture_parameters() const;

    int width;
    int height;
    int channels;
    std::vector<uint8_t> data;
    GLuint texture = 0;

private:
    void bind(GLuint slot = 0) const;
    void upload();
};