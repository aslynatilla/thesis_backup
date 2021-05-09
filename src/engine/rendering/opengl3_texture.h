#ifndef OPENGL3_TEXTURE_H
#define OPENGL3_TEXTURE_H

#include <glad/glad.h>
#include <iterator>
#include <vector>

namespace engine{
    struct OpenGL3_TextureParameters{
    public:
        struct OpenGL3_TextureParameter_Pair{
            unsigned int name;
            unsigned int value;
        };

        OpenGL3_TextureParameters(std::initializer_list<GLenum> names, std::initializer_list<GLenum> values);
        std::vector<OpenGL3_TextureParameter_Pair>::const_iterator begin() const;
        std::vector<OpenGL3_TextureParameter_Pair>::const_iterator end() const;

    private:
        std::vector<OpenGL3_TextureParameter_Pair> parameters;
    };


    class OpenGL3_Texture2D {
    public:
        OpenGL3_Texture2D(const GLenum image_format, const OpenGL3_TextureParameters& parameters,
                          const unsigned int texture_width, const unsigned int texture_height,
                          const GLenum data_format, const GLenum data_type,
                          const void* data);
        ~OpenGL3_Texture2D();
        OpenGL3_Texture2D(const OpenGL3_Texture2D& other) = delete;
        OpenGL3_Texture2D(OpenGL3_Texture2D&& other) = delete;
        OpenGL3_Texture2D& operator=(const OpenGL3_Texture2D& other) = delete;
        OpenGL3_Texture2D& operator=(OpenGL3_Texture2D&& other) = delete;

        void bind_to_slot(const unsigned int slot);

        unsigned int id;
        GLenum bound_type;
        unsigned int width;
        unsigned int height;
    };


    class OpenGL3_Texture1D {
    public:
        OpenGL3_Texture1D(const GLenum image_format, const OpenGL3_TextureParameters& parameters,
                          const unsigned int texture_width, const GLenum data_format,
                          const GLenum data_type, const void* data);
        ~OpenGL3_Texture1D();
        OpenGL3_Texture1D(const OpenGL3_Texture1D& other) = delete;
        OpenGL3_Texture1D(OpenGL3_Texture1D&& other) = delete;
        OpenGL3_Texture1D& operator=(const OpenGL3_Texture1D& other) = delete;
        OpenGL3_Texture1D& operator=(OpenGL3_Texture1D&& other) = delete;

        void bind_to_slot(const unsigned int slot);

        unsigned int id;
        GLenum bound_type;
        unsigned int width;
    };
}

#endif //OPENGL3_TEXTURE_H
