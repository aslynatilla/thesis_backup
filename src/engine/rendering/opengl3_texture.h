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


    class OpenGL3_Texture {
    public:
        OpenGL3_Texture(const GLenum texture_type, const GLenum image_format,
                        const OpenGL3_TextureParameters& parameters,
                        const unsigned int texture_width, const unsigned int texture_height,
                        const GLenum data_format, const GLenum data_type, const void* data);
        ~OpenGL3_Texture();
        OpenGL3_Texture(const OpenGL3_Texture& other) = delete;
        OpenGL3_Texture(OpenGL3_Texture&& other) = delete;
        OpenGL3_Texture& operator=(const OpenGL3_Texture& other) = delete;
        OpenGL3_Texture& operator=(OpenGL3_Texture&& other) = delete;

        void make_active_in_slot(const unsigned int slot);

        unsigned int id;
        GLenum bound_type;
        unsigned int width;
        unsigned int height;
    };
}

#endif //OPENGL3_TEXTURE_H
