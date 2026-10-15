#include "Pyramid.hpp"

#include "Texture.hpp"

#include <glad/glad.h>
#include <glm/geometric.hpp>
#include <glm/vec3.hpp>

#include <stdexcept>
#include <vector>

namespace
{
std::vector<float> makePyramidVertices()
{
    const glm::vec3 apex(0.0f, 0.7f, 0.0f);
    const glm::vec3 base[] = {
        {-0.5f, -0.5f, -0.5f},
        { 0.5f, -0.5f, -0.5f},
        { 0.5f, -0.5f,  0.5f},
        {-0.5f, -0.5f,  0.5f}
    };
    std::vector<float> vertices;
    vertices.reserve(18 * 8);

    const auto append = [&vertices](const glm::vec3& position, const glm::vec3& normal, float u, float v)
    {
        vertices.insert(
            vertices.end(),
            {position.x, position.y, position.z, normal.x, normal.y, normal.z, u, v});
    };

    for (int side = 0; side < 4; ++side)
    {
        const glm::vec3& left = base[side];
        const glm::vec3& right = base[(side + 1) % 4];
        const glm::vec3 normal = glm::normalize(glm::cross(apex - left, right - left));
        append(left, normal, 0.0f, 0.0f);
        append(right, normal, 1.0f, 0.0f);
        append(apex, normal, 0.5f, 1.0f);
    }

    const glm::vec3 bottomNormal(0.0f, -1.0f, 0.0f);
    append(base[0], bottomNormal, 0.0f, 0.0f);
    append(base[2], bottomNormal, 1.0f, 1.0f);
    append(base[1], bottomNormal, 1.0f, 0.0f);
    append(base[0], bottomNormal, 0.0f, 0.0f);
    append(base[3], bottomNormal, 0.0f, 1.0f);
    append(base[2], bottomNormal, 1.0f, 1.0f);

    return vertices;
}
}

Pyramid::Pyramid()
{
    const std::vector<float> vertices = makePyramidVertices();
    glGenVertexArrays(1, &m_vertexArray);
    glGenBuffers(1, &m_vertexBuffer);
    glBindVertexArray(m_vertexArray);
    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);
    glBufferData(
        GL_ARRAY_BUFFER,
        static_cast<GLsizeiptr>(vertices.size() * sizeof(float)),
        vertices.data(),
        GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
        reinterpret_cast<void*>(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
        reinterpret_cast<void*>(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

Pyramid::~Pyramid()
{
    destroy();
}

void Pyramid::setTexture(Texture& texture)
{
    if (m_destroyed || texture.isDestroyed())
    {
        throw std::invalid_argument("Cannot assign a texture to a destroyed pyramid.");
    }
    m_texture = &texture;
}

void Pyramid::draw() const
{
    if (m_destroyed)
    {
        throw std::logic_error("Cannot draw a destroyed pyramid.");
    }
    if (m_texture != nullptr)
    {
        m_texture->bind();
    }
    glBindVertexArray(m_vertexArray);
    glDrawArrays(GL_TRIANGLES, 0, 18);
    glBindVertexArray(0);
}

void Pyramid::destroy()
{
    if (m_vertexBuffer != 0)
    {
        glDeleteBuffers(1, &m_vertexBuffer);
        m_vertexBuffer = 0;
    }
    if (m_vertexArray != 0)
    {
        glDeleteVertexArrays(1, &m_vertexArray);
        m_vertexArray = 0;
    }
    m_destroyed = true;
}

bool Pyramid::isDestroyed() const
{
    return m_destroyed;
}
