#ifndef IES_PARSER_SHADER_H
#define IES_PARSER_SHADER_H

#include <fmt/core.h>
#include <fmt/color.h>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <string_view>
#include <array>
#include <algorithm>

namespace engine {
    class Shader {
    public:
        unsigned int id;
        bool successfully_built;

        Shader(std::string&& vertex_src, std::string&& fragment_src,
               const std::string_view vertex_name = "vertex", const std::string_view fragment_name = "fragment");
        Shader(std::string&& vertex_src, std::string&& geometry_src, std::string&& fragment_src,
               const std::string_view vertex_name = "vertex",
               const std::string_view geometry_name = "geometry",
               const std::string_view fragment_name = "fragment");

        void use() const;
        void set_int(const std::string_view name, const int val) const noexcept;
        void set_bool(const std::string_view name, const bool val) const noexcept;
        void set_vec2(const std::string_view name, const glm::vec2 v) const noexcept;
        void set_vec2(const std::string_view name, const float vx, const float vy) const noexcept;
        void set_vec3(const std::string_view name, const glm::vec3 v) const noexcept;
        void set_vec3(const std::string_view name, const float vx, const float vy, const float vz) const noexcept;
        void set_vec4(const std::string_view name, const glm::vec4 v) const noexcept;
        void set_vec4(const std::string_view name, const float vx, const float vy, const float vz, const float w) const noexcept;
        void set_mat4(const std::string_view name, const glm::mat4 mat) const noexcept;
        void set_uint(const std::string_view name, const unsigned int val) const noexcept;
        void set_float(const std::string_view name, const float val) const noexcept;

    private:
        struct shader_object_src{
            std::string_view source;
            std::string_view name;
            unsigned int type;
        };

        template <int sources_size>
        void initialize_from_sources(const std::array<shader_object_src, sources_size>& sources);

        static unsigned int shader_object_from(const char* source, const unsigned int gl_shader_type);
        static bool         check_shader_object_compilation(const unsigned int shader_id, const std::string_view shader_name);
        static bool         check_shader_program_linking(const unsigned int shader_id);

    };
}

#endif //IES_PARSER_SHADER_H
