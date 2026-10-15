#pragma once

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

#include <string>

class Shader
{
public:
    static constexpr unsigned int MaxLights = 8;

    Shader(const std::string& vertexPath, const std::string& fragmentPath);
    ~Shader();

    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;

    void use() const;
    void setMat4(const std::string& name, const glm::mat4& value) const;
    void setVec3(const std::string& name, const glm::vec3& value) const;
    void setFloat(const std::string& name, float value) const;
    void setInt(const std::string& name, int value) const;
    void destroy();
    bool isDestroyed() const;

private:
    unsigned int m_program = 0;
    bool m_destroyed = false;
};
