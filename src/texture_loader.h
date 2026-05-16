#pragma once
#include <filesystem>
#include <vector>
#include "vendor/glad/include/glad/glad.h"
#include "vendor/glfw/include/GLFW/glfw3.h"

namespace fs = std::filesystem;

class LoadedImage {
public:
    void initialize(GLuint slot = 0) {
        bind(slot);
        set_texture_parameters();
        upload();
    }

    void resend() {
        bind();
        upload();
    }

    void destroy() {
        if (texture != 0) {
            glDeleteTextures(1, &texture);
            texture = 0;
        }
    }

    void set_texture_parameters() const {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }

    int width;
    int height;
    int channels;
    std::vector<uint8_t> data;
    GLuint texture = 0;

private:
    void bind(GLuint slot = 0) const {
        glActiveTexture(GL_TEXTURE0 + slot);
        glBindTexture(GL_TEXTURE_2D, texture);
    }

    void upload() {
        if (channels != 4) {
            throw std::runtime_error("LoadedImage must contain RGBA data");
        }

        if (texture == 0) {
            glGenTextures(1, &texture);
        }

        bind();

        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RGBA,
            width,
            height,
            0,
            GL_RGBA,
            GL_UNSIGNED_BYTE,
            data.data()
        );
    }
};

LoadedImage load_png_rgba(const std::string& path);