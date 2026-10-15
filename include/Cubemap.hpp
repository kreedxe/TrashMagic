#pragma once

#include <string>

class Cubemap
{
public:
    explicit Cubemap(const std::string& crossImagePath);
    ~Cubemap();

    Cubemap(const Cubemap&) = delete;
    Cubemap& operator=(const Cubemap&) = delete;

    void bind(unsigned int textureUnit = 0) const;
    void destroy();
    bool isDestroyed() const;

private:
    unsigned int m_texture = 0;
    bool m_destroyed = false;
};
