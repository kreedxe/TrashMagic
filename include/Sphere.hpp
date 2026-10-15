#pragma once

class Texture;

class Sphere
{
public:
    Sphere(unsigned int longitudeSegments = 32, unsigned int latitudeSegments = 16);
    ~Sphere();

    Sphere(const Sphere&) = delete;
    Sphere& operator=(const Sphere&) = delete;

    void setTexture(Texture& texture);
    void draw() const;
    void destroy();
    bool isDestroyed() const;

private:
    unsigned int m_vertexArray = 0;
    unsigned int m_vertexBuffer = 0;
    int m_vertexCount = 0;
    Texture* m_texture = nullptr;
    bool m_destroyed = false;
};
