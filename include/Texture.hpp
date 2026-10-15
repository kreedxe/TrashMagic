#pragma once

#include <string>

class Texture
{
public:
    explicit Texture(const std::string& path);
    ~Texture();

    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;

    void bind(unsigned int textureUnit = 0) const;
    void destroy();
    bool isDestroyed() const;

private:
    unsigned int m_texture = 0;
    bool m_destroyed = false;
};
