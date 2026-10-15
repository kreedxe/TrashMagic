#include "Shader.hpp"

#include <glad/glad.h>

#include <glm/gtc/type_ptr.hpp>

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>

namespace
{
std::string readShaderFile(const std::string& path)
{
    std::ifstream file(path);
    if (!file)
    {
        throw std::runtime_error("Failed to open shader file: " + path);
    }

    std::ostringstream contents;
    contents << file.rdbuf();
    return contents.str();
}

GLuint compileShader(GLenum type, const std::string& source, const std::string& path)
{
    const GLuint shader = glCreateShader(type);
    const char* sourceData = source.c_str();
    glShaderSource(shader, 1, &sourceData, nullptr);
    glCompileShader(shader);

    GLint compiled = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if (compiled == GL_FALSE)
    {
        GLint logLength = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
        std::string log(static_cast<std::size_t>(logLength > 0 ? logLength : 1), '\0');
        glGetShaderInfoLog(shader, static_cast<GLsizei>(log.size()), nullptr, &log[0]);
        glDeleteShader(shader);
        throw std::runtime_error("Failed to compile shader '" + path + "':\n" + log);
    }

    return shader;
}
}

Shader::Shader(const std::string& vertexPath, const std::string& fragmentPath)
{
    const GLuint vertexShader = compileShader(
        GL_VERTEX_SHADER, readShaderFile(vertexPath), vertexPath);
    GLuint fragmentShader = 0;

    try
    {
        fragmentShader = compileShader(
            GL_FRAGMENT_SHADER, readShaderFile(fragmentPath), fragmentPath);
        m_program = glCreateProgram();
        glAttachShader(m_program, vertexShader);
        glAttachShader(m_program, fragmentShader);
        glLinkProgram(m_program);

        GLint linked = GL_FALSE;
        glGetProgramiv(m_program, GL_LINK_STATUS, &linked);
        if (linked == GL_FALSE)
        {
            GLint logLength = 0;
            glGetProgramiv(m_program, GL_INFO_LOG_LENGTH, &logLength);
            std::string log(static_cast<std::size_t>(logLength > 0 ? logLength : 1), '\0');
            glGetProgramInfoLog(m_program, static_cast<GLsizei>(log.size()), nullptr, &log[0]);
            glDeleteProgram(m_program);
            m_program = 0;
            throw std::runtime_error("Failed to link shader program:\n" + log);
        }
    }
    catch (...)
    {
        if (fragmentShader != 0)
        {
            glDeleteShader(fragmentShader);
        }
        glDeleteShader(vertexShader);
        throw;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

Shader::~Shader()
{
    destroy();
}

void Shader::use() const
{
    if (m_destroyed)
    {
        throw std::logic_error("Cannot use a destroyed shader.");
    }
    glUseProgram(m_program);
}

void Shader::setMat4(const std::string& name, const glm::mat4& value) const
{
    if (m_destroyed)
    {
        throw std::logic_error("Cannot update a destroyed shader.");
    }
    const GLint location = glGetUniformLocation(m_program, name.c_str());
    glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(value));
}

void Shader::setVec3(const std::string& name, const glm::vec3& value) const
{
    if (m_destroyed)
    {
        throw std::logic_error("Cannot update a destroyed shader.");
    }
    const GLint location = glGetUniformLocation(m_program, name.c_str());
    glUniform3fv(location, 1, glm::value_ptr(value));
}

void Shader::setFloat(const std::string& name, float value) const
{
    if (m_destroyed)
    {
        throw std::logic_error("Cannot update a destroyed shader.");
    }
    const GLint location = glGetUniformLocation(m_program, name.c_str());
    glUniform1f(location, value);
}

void Shader::setInt(const std::string& name, int value) const
{
    if (m_destroyed)
    {
        throw std::logic_error("Cannot update a destroyed shader.");
    }
    const GLint location = glGetUniformLocation(m_program, name.c_str());
    glUniform1i(location, value);
}

void Shader::destroy()
{
    if (m_program != 0)
    {
        glDeleteProgram(m_program);
        m_program = 0;
    }
    m_destroyed = true;
}

bool Shader::isDestroyed() const
{
    return m_destroyed;
}
