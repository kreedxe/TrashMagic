#include "Sphere.hpp"

#include "Texture.hpp"

#include <glad/glad.h>
#include <glm/vec3.hpp>

#include <cmath>
#include <stdexcept>
#include <vector>

namespace
{
constexpr float Pi = 3.14159265358979323846f;

void appendVertex(std::vector<float>& vertices, const glm::vec3& position, float u, float v)
{
    vertices.insert(
        vertices.end(),
        {position.x, position.y, position.z, position.x, position.y, position.z, u, v});
}
}

Sphere::Sphere(unsigned int longitudeSegments, unsigned int latitudeSegments)
{
    if (longitudeSegments < 3 || latitudeSegments < 2)
    {
        throw std::invalid_argument("Sphere requires at least 3 longitude and 2 latitude segments.");
    }

    std::vector<float> vertices;
    vertices.reserve(
        static_cast<std::size_t>(longitudeSegments) * latitudeSegments * 6 * 8);

    for (unsigned int latitude = 0; latitude < latitudeSegments; ++latitude)
    {
        const float v0 = static_cast<float>(latitude) / latitudeSegments;
        const float v1 = static_cast<float>(latitude + 1) / latitudeSegments;
        const float phi0 = Pi * (v0 - 0.5f);
        const float phi1 = Pi * (v1 - 0.5f);

        for (unsigned int longitude = 0; longitude < longitudeSegments; ++longitude)
        {
            const float u0 = static_cast<float>(longitude) / longitudeSegments;
            const float u1 = static_cast<float>(longitude + 1) / longitudeSegments;
            const float theta0 = 2.0f * Pi * u0;
            const float theta1 = 2.0f * Pi * u1;

            const auto point = [](float theta, float phi)
            {
                const float cosPhi = std::cos(phi);
                return glm::vec3(
                    cosPhi * std::cos(theta),
                    std::sin(phi),
                    cosPhi * std::sin(theta));
            };

            const glm::vec3 a = point(theta0, phi0);
            const glm::vec3 b = point(theta0, phi1);
            const glm::vec3 c = point(theta1, phi1);
            const glm::vec3 d = point(theta1, phi0);

            appendVertex(vertices, a, u0, v0);
            appendVertex(vertices, b, u0, v1);
            appendVertex(vertices, c, u1, v1);
            appendVertex(vertices, a, u0, v0);
            appendVertex(vertices, c, u1, v1);
            appendVertex(vertices, d, u1, v0);
        }
    }

    m_vertexCount = static_cast<int>(vertices.size() / 8);
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

Sphere::~Sphere()
{
    destroy();
}

void Sphere::setTexture(Texture& texture)
{
    if (m_destroyed || texture.isDestroyed())
    {
        throw std::invalid_argument("Cannot assign a texture to a destroyed sphere.");
    }
    m_texture = &texture;
}

void Sphere::draw() const
{
    if (m_destroyed)
    {
        throw std::logic_error("Cannot draw a destroyed sphere.");
    }
    if (m_texture != nullptr)
    {
        m_texture->bind();
    }
    glBindVertexArray(m_vertexArray);
    glDrawArrays(GL_TRIANGLES, 0, m_vertexCount);
    glBindVertexArray(0);
}

void Sphere::destroy()
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

bool Sphere::isDestroyed() const
{
    return m_destroyed;
}
