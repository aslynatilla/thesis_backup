
#include "opengl3_cubemap.h"
namespace engine{
    OpenGL3_Cubemap::OpenGL3_Cubemap(const GLenum image_format,
                                             const OpenGL3_TextureParameters& parameters,
                                             const unsigned int texture_width, const unsigned int texture_height,
                                             const GLenum data_format, const GLenum data_type, const void* data) {
        id = 0;
        glGenTextures(1, &id);
        glBindTexture(GL_TEXTURE_CUBE_MAP, id);
        for(auto i = 0; i < 6; ++i){
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, image_format, texture_width, texture_height, 0, data_format, data_type, data);
        }

        for(const auto parameter_pair : parameters){
            glTexParameteri(GL_TEXTURE_CUBE_MAP, parameter_pair.name, parameter_pair.value);
        }

        //glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
        bound_type = GL_TEXTURE_CUBE_MAP;
        width = texture_width;
        height = texture_height;
    }

    OpenGL3_Cubemap::~OpenGL3_Cubemap() {
        glDeleteTextures(1, &id);
    }

    void OpenGL3_Cubemap::make_active_in_slot(const unsigned int slot) {
        glActiveTexture(GL_TEXTURE0+slot);
        glBindTexture(bound_type, id);
    }
}

