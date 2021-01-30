#include "shader.h"

namespace engine{
    Shader::Shader(std::string&& vertex_src, std::string&& fragment_src, const std::string_view vertex_name,
                   const std::string_view fragment_name) {
        const std::array<shader_object_src, 2> sources{
                shader_object_src{vertex_src, vertex_name, GL_VERTEX_SHADER},
                shader_object_src{fragment_src, fragment_name, GL_FRAGMENT_SHADER}
        };

        initialize_from_sources<2>(sources);
    }



    Shader::Shader(std::string&& vertex_src, std::string&& geometry_src, std::string&& fragment_src,
                   const std::string_view vertex_name, const std::string_view geometry_name,
                   const std::string_view fragment_name) {
        const std::array<shader_object_src, 3> sources{
                shader_object_src{vertex_src, vertex_name, GL_VERTEX_SHADER},
                shader_object_src{geometry_src, geometry_name, GL_GEOMETRY_SHADER},
                shader_object_src{fragment_src, fragment_name, GL_FRAGMENT_SHADER}
        };

        initialize_from_sources<3>(sources);
    }

    void Shader::use() const
    {
        glUseProgram(this->id);
    }

    void Shader::set_int(const std::string_view name, const int val) const noexcept
    {
        glUniform1i(glGetUniformLocation(id, name.data()), val);
    }

    void Shader::set_bool(const std::string_view name, const bool val) const noexcept
    {
        glUniform1i(glGetUniformLocation(id, name.data()), static_cast<int>(val));
    }

    void Shader::set_vec2(const std::string_view name, const glm::vec2 v) const noexcept
    {
        glUniform2f(glGetUniformLocation(id, name.data()), v.x, v.y);
    }

    void Shader::set_vec2(const std::string_view name, const float vx, const float vy) const noexcept
    {
        glUniform2f(glGetUniformLocation(id, name.data()), vx, vy);
    }

    void Shader::set_vec3(const std::string_view name, const glm::vec3 v) const noexcept
    {
        glUniform3f(glGetUniformLocation(id, name.data()), v.x, v.y, v.z);
    }

    void Shader::set_vec3(const std::string_view name, const float vx, const float vy, const float vz) const noexcept
    {
        glUniform3f(glGetUniformLocation(id, name.data()), vx, vy, vz);
    }

    void Shader::set_vec4(const std::string_view name, const glm::vec4 v) const noexcept
    {
        glUniform4f(glGetUniformLocation(id, name.data()), v.x, v.y, v.z, v.w);
    }

    void Shader::set_vec4(const std::string_view name, const float vx, const float vy, const float vz, const float vw) const noexcept
    {
        glUniform4f(glGetUniformLocation(id, name.data()), vx, vy, vz, vw);
    }

    void Shader::set_mat4(const std::string_view name, const glm::mat4 mat) const noexcept{
        glUniformMatrix4fv(glGetUniformLocation(id, name.data()), 1, GL_FALSE, glm::value_ptr(mat));
    }

    void Shader::set_uint(const std::string_view name, const unsigned int val) const noexcept
    {
        glUniform1ui(glGetUniformLocation(id, name.data()), val);
    }

    void Shader::set_float(const std::string_view name, const float val) const noexcept
    {
        glUniform1f(glGetUniformLocation(id, name.data()), val);
    }

    template<int source_size>
    void Shader::initialize_from_sources(const std::array<shader_object_src, source_size>& sources) {
        std::array<unsigned int, source_size> shader_objects;

        successfully_built = true;
        std::transform(std::begin(sources), std::end(sources), std::begin(shader_objects), [this](const auto& s) {
            const auto shader_id = this->shader_object_from(s.source.data(), s.type);
            successfully_built &= check_shader_object_compilation(shader_id, s.name);
            return shader_id;
        });

        id = glCreateProgram();

        for (const auto shader_object : shader_objects) {
            glAttachShader(id, shader_object);
        }

        glLinkProgram(id);
        successfully_built &= check_shader_program_linking(id);
        for (const auto shader_object : shader_objects) {
            glDeleteShader(shader_object);
        }
    }

    unsigned int Shader::shader_object_from(const char* source, const unsigned int gl_shader_type) {
        const unsigned int obj_id = glCreateShader(gl_shader_type);
        glShaderSource(obj_id, 1, &source, nullptr);
        glCompileShader(obj_id);
        return obj_id;
    }

    bool Shader::check_shader_object_compilation(const unsigned int shader_id, const std::string_view shader_name) {
        int  success;
        char log[512];

        glGetShaderiv(shader_id, GL_COMPILE_STATUS, &success);
        if (success == 0) {
            glGetShaderInfoLog(shader_id, 512, nullptr, log);
            fmt::print("SHADER >>> Compilation failed for shader with id: {} named {}\nInfo log >>> {}", shader_id, shader_name, log);
            return false;
        } else {
            return true;
        }
    }

    bool Shader::check_shader_program_linking(const unsigned int shader_id) {
        int  success;
        char log[512];

        glGetProgramiv(shader_id, GL_LINK_STATUS, &success);
        if (success == 0) {
            glGetProgramInfoLog(shader_id, 512, nullptr, log);
            fmt::print("SHADER >>> Linking failed for shader program with id: {}\nInfo log >>> {}", shader_id, log);
            return false;
        }
        return true;
    }
}