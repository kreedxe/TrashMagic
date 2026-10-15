#pragma once

class Texture;

class Cube
{
public:
    Cube();
    ~Cube();

    Cube(const Cube&) = delete;
    Cube& operator=(const Cube&) = delete;

    void setTexture(Texture& texture);
    void draw() const;
    void destroy();
    bool isDestroyed() const;

private:
    unsigned int m_vertexArray = 0;
    unsigned int m_vertexBuffer = 0;
    Texture* m_texture = nullptr;
    bool m_destroyed = false;
};
