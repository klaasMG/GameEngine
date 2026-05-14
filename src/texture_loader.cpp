#include "texture_loader.h"
#define STB_IMAGE_IMPLEMENTATION
#include "vendor/stb_image/stb_image.h"

LoadedImage load_png_rgba(const std::string& path) {
    int width, height, channels;

    // Force RGBA output
    unsigned char* pixels = stbi_load(
        path.c_str(),
        &width,
        &height,
        &channels,
        STBI_rgb_alpha
    );

    if (!pixels) {
        throw std::runtime_error(
            std::string("Failed to load image: ") + stbi_failure_reason()
        );
    }

    size_t size = static_cast<size_t>(width) * height * 4;

    LoadedImage image;
    image.width = width;
    image.height = height;
    image.channels = 4;
    image.data.assign(pixels, pixels + size);

    stbi_image_free(pixels);

    return image;
}

LoadedImage load_textures(const fs::path& path) {
    LoadedImage image = load_png_rgba(path.string());
    if (image.height % 16 != 0 || image.width % 16 != 0) {
        throw std::runtime_error("the image must be a multipule of 16 in width and height");
    }
    return std::move(image);
}
