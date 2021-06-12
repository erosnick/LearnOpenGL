#include "Texture.h"

#include <imgui/stb_image.h>
#include <memory>
#include <iostream>
#include <filesystem>

uint32_t Texture::activeIndex = 0;

std::string Texture::suffixes[] = { "posx", "negx", "posy", "negy", "posz", "negz" };

Texture::Texture(const std::string& name, int32_t inWidth, int32_t inHeight, int32_t filter) {

    width = inWidth;
    height = inHeight;

    glGenTextures(1, &id);
    glActiveTexture(GL_TEXTURE0 + activeIndex++);
    glBindTexture(GL_TEXTURE_2D, id);

    uint8_t* data = new uint8_t[width * height * 4];

    memset(data, 255, width * height * 4);
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

    delete[] data;

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
}

void Texture::load(const std::string& fileName, int32_t wrapMode) {
    if (!std::filesystem::exists(fileName)) {
        std::cout << "File " + fileName << " doesn't exists.\n";
        return;
    }

    int32_t bpp = 0;

    uint8_t* data = stbi_load(fileName.c_str(), &width, &height, &bpp, 4);

    glActiveTexture(GL_TEXTURE0 + activeIndex++);

    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapMode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapMode);

    stbi_image_free(data);
}

void Texture::loadCubeMap(const std::string& baseName, int32_t wrapMode, bool hdr) {
    glActiveTexture(GL_TEXTURE0 + activeIndex++);

    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_CUBE_MAP, id);

    uint32_t targets[] = {
        GL_TEXTURE_CUBE_MAP_POSITIVE_X,
        GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
        GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
        GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
        GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
        GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
    };

    for (int i = 0; i < 6; i++) {
        int32_t width = 0;
        int32_t height = 0;
        int32_t bpp = 0;

        std::string ext = ".png";
        
        if (hdr) {
            ext = ".hdr";
        }

        std::string textureName = std::string(baseName) + "_" + suffixes[i] + ext;

        if (!std::filesystem::exists(textureName)) {
            std::cout << "Texture " + textureName + " not found.\n";
            return;
        }

        uint8_t* data = stbi_load(textureName.c_str(), &width, &height, &bpp, 4);

        glTexImage2D(targets[i], 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

        stbi_image_free(data);
    }

    glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, wrapMode);
    glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, wrapMode);
    glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, wrapMode);
}

void Texture::use() {
    glBindTexture(GL_TEXTURE_2D, id);
}
