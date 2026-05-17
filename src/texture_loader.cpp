#define STB_IMAGE_IMPLEMENTATION
#include "texture_loader.h"

#include <iostream>

LoadedImage::LoadedImage(const std::string& path, GLuint slot) {
    int loaded_width, loaded_height, loaded_channels;

    unsigned char* pixels = stbi_load(
        path.c_str(),
        &loaded_width,
        &loaded_height,
        &loaded_channels,
        STBI_rgb_alpha
    );

    if (!pixels) {
        throw std::runtime_error(
            std::string("Failed to load image: ") + stbi_failure_reason()
        );
    }

    if (loaded_width % 16 != 0 || loaded_height % 16 != 0) {
        stbi_image_free(pixels);
        throw std::runtime_error("the image must be a multipule of 16 in width and height");
    }

    width = loaded_width;
    height = loaded_height;
    channels = 4;

    size_t size = static_cast<size_t>(width) * height * 4;
    data.assign(pixels, pixels + size);
    size_t index = 0;
    for (uint8_t pixel : data) {
        index++;
        //std::cout << "index:" << index << "data:" << pixel << std::endl;
    }

    stbi_image_free(pixels);


    if (texture == 0) {
        glGenTextures(1, &texture);
    }

    bind(slot);
    set_texture_parameters();
    upload();
    std::cout << "Loaded image: " << path << height << width<< std::endl;
}

void LoadedImage::resend() {
    bind();
    upload();
}

void LoadedImage::destroy() {
    if (texture != 0) {
        glDeleteTextures(1, &texture);
        texture = 0;
    }
}

void LoadedImage::set_texture_parameters() const {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

void LoadedImage::bind(GLuint slot) const {
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, texture);
}

void LoadedImage::upload() {
    if (channels != 4) {
        throw std::runtime_error("LoadedImage must contain RGBA data");
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