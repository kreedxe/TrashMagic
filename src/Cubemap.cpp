#include "Cubemap.hpp"

#include <glad/glad.h>

#include "stb_image.h"

#include <algorithm>
#include <cstddef>
#include <stdexcept>
#include <string>
#include <vector>

namespace
{
constexpr int faceColumns[] = {2, 0, 1, 1, 1, 3};
constexpr int faceRows[] = {1, 1, 0, 2, 1, 1};
}

Cubemap::Cubemap(const std::string& crossImagePath)
{
    stbi_set_flip_vertically_on_load(false);

    int width = 0;
    int height = 0;
    int channels = 0;
    unsigned char* source = stbi_load(
        crossImagePath.c_str(), &width, &height, &channels, STBI_rgb_alpha);
    if (source == nullptr)
    {
        const char* reason = stbi_failure_reason();
        throw std::runtime_error(
            "Failed to load cubemap '" + crossImagePath + "': "
            + (reason != nullptr ? reason : "unknown image error"));
    }

    if (width % 4 != 0 || height % 3 != 0 || width / 4 != height / 3)
    {
        stbi_image_free(source);
        throw std::runtime_error(
            "Cubemap cross must have a 4-by-3 layout with square faces: " + crossImagePath);
    }

    const int faceSize = width / 4;
    std::vector<unsigned char> facePixels(
        static_cast<std::size_t>(faceSize) * static_cast<std::size_t>(faceSize) * 4);

    glGenTextures(1, &m_texture);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_texture);
    for (int face = 0; face < 6; ++face)
    {
        const int startX = faceColumns[face] * faceSize;
        const int startY = faceRows[face] * faceSize;
        for (int y = 0; y < faceSize; ++y)
        {
            const auto sourceOffset =
                (static_cast<std::size_t>(startY + y) * width + startX) * 4;
            const auto destinationOffset = static_cast<std::size_t>(y) * faceSize * 4;
            std::copy_n(
                source + sourceOffset,
                static_cast<std::size_t>(faceSize) * 4,
                facePixels.begin() + destinationOffset);
        }

        glTexImage2D(
            GL_TEXTURE_CUBE_MAP_POSITIVE_X + face,
            0,
            GL_SRGB8_ALPHA8,
            faceSize,
            faceSize,
            0,
            GL_RGBA,
            GL_UNSIGNED_BYTE,
            facePixels.data());
    }
    stbi_image_free(source);

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

Cubemap::~Cubemap()
{
    destroy();
}

void Cubemap::bind(unsigned int textureUnit) const
{
    if (m_destroyed)
    {
        throw std::logic_error("Cannot bind a destroyed cubemap.");
    }
    glActiveTexture(GL_TEXTURE0 + textureUnit);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_texture);
}

void Cubemap::destroy()
{
    if (m_texture != 0)
    {
        glDeleteTextures(1, &m_texture);
        m_texture = 0;
    }
    m_destroyed = true;
}

bool Cubemap::isDestroyed() const
{
    return m_destroyed;
}
