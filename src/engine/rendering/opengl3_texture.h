#ifndef OPENGL3_TEXTURE_H
#define OPENGL3_TEXTURE_H

#include <glad/glad.h>
#include <array>
#include <iterator>
#include <unordered_map>
#include <memory>

namespace engine {
    struct OpenGL3_TextureParametersInts {
        using container = std::unordered_map<GLenum, GLint>;
        using insertion_result = std::pair<container::iterator, bool>;
    public:
        [[nodiscard]] container::const_iterator begin() const;
        [[nodiscard]] container::const_iterator end() const;

        insertion_result add_parameter(GLenum name, GLenum value);
    private:
        container parameters;
    };

    class OpenGL3_Texture2D {
    public:
        friend class OpenGL3_Texture2D_Builder;
        void bind_to_slot(unsigned int slot) const;
        [[nodiscard]] unsigned int id() const;

        ~OpenGL3_Texture2D();

        OpenGL3_Texture2D(const OpenGL3_Texture2D& other) = delete;
        OpenGL3_Texture2D(OpenGL3_Texture2D&& other) = delete;
        OpenGL3_Texture2D& operator=(const OpenGL3_Texture2D& other) = delete;
        OpenGL3_Texture2D& operator=(OpenGL3_Texture2D&& other) = delete;
    private:
        OpenGL3_Texture2D(unsigned int texture_id,
                          unsigned int texture_width, unsigned int texture_height);

        unsigned int resource_id;
        GLenum bound_type;
        unsigned int width;
        unsigned int height;
    };


    class OpenGL3_Texture1D {
    public:
        friend class OpenGL3_Texture1D_Builder;
        void bind_to_slot(unsigned int slot) const;
        [[nodiscard]] unsigned int id() const;

        ~OpenGL3_Texture1D();

        OpenGL3_Texture1D(const OpenGL3_Texture1D& other) = delete;
        OpenGL3_Texture1D(OpenGL3_Texture1D&& other) = delete;
        OpenGL3_Texture1D& operator=(const OpenGL3_Texture1D& other) = delete;
        OpenGL3_Texture1D& operator=(OpenGL3_Texture1D&& other) = delete;
    private:
        OpenGL3_Texture1D(unsigned int texture_id,
                          int texture_dimension);

        unsigned int resource_id;
        GLenum bound_type;
        int dimension;
    };

    template <typename T>
    class OpenGL3_Texture_Builder{
    public:
        [[nodiscard]] T&& with_texture_format(GLint tex_format) && {
            texture_format = tex_format;
            return static_cast<T&&>(*this);
        }

        [[nodiscard]] T&& with_data_format(GLenum data_format) && {
            texture_data_format = data_format;
            return static_cast<T&&>(*this);
        }

        [[nodiscard]] T&& using_underlying_data_type(GLenum underlying_type) && {
            data_type = underlying_type;
            return static_cast<T&&>(*this);
        }

        [[nodiscard]] T&& using_linear_minification() && {
            parameters.add_parameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            return static_cast<T&&>(*this);
        }

        [[nodiscard]] T&& using_nearest_minification() && {
            parameters.add_parameter(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            return static_cast<T&&>(*this);
        }

        [[nodiscard]] T&& using_linear_magnification() && {
            parameters.add_parameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            return static_cast<T&&>(*this);
        }

        [[nodiscard]] T&& using_nearest_magnification() && {
            parameters.add_parameter(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            return static_cast<T&&>(*this);
        }

    protected:
        unsigned int id = 0;
        GLint texture_format = 0;
        GLenum texture_data_format = 0;
        GLenum data_type = 0;
        OpenGL3_TextureParametersInts parameters;
    };

    class OpenGL3_Texture2D_Builder : public OpenGL3_Texture_Builder<OpenGL3_Texture2D_Builder>{
    public:
        OpenGL3_Texture2D_Builder() = default;
        [[nodiscard]] OpenGL3_Texture2D_Builder&& with_size(int tex_width, int tex_height) &&;
        [[nodiscard]] OpenGL3_Texture2D_Builder&& using_border_color(const std::array<float, 4>& color) &&;
        [[nodiscard]] OpenGL3_Texture2D_Builder&& using_clamping_to_borders() &&;
        [[nodiscard]] OpenGL3_Texture2D_Builder&& using_clamping_to_edge() &&;

        [[nodiscard]] std::unique_ptr<OpenGL3_Texture2D> as_resource();
        [[nodiscard]] std::unique_ptr<OpenGL3_Texture2D> as_resource_with_data(const void* data);

    private:
        int width = 0;
        int height = 0;
        std::array<float, 4> border_color{};
    };

    class OpenGL3_Texture1D_Builder : public OpenGL3_Texture_Builder<OpenGL3_Texture1D_Builder>{
    public:
        OpenGL3_Texture1D_Builder() = default;
        [[nodiscard]] OpenGL3_Texture1D_Builder&& with_size(int size) &&;
        [[nodiscard]] OpenGL3_Texture1D_Builder&& using_clamping_to_borders() &&;
        [[nodiscard]] OpenGL3_Texture1D_Builder&& using_clamping_to_edge() &&;

        [[nodiscard]] std::unique_ptr<OpenGL3_Texture1D> as_resource();
        [[nodiscard]] std::unique_ptr<OpenGL3_Texture1D> as_resource_with_data(const void* data);
    private:
        int dimension = 0;
    };
}

#endif //OPENGL3_TEXTURE_H
