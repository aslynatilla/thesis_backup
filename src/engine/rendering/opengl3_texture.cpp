#include "opengl3_texture.h"

namespace engine{

    OpenGL3_TextureParameters::OpenGL3_TextureParameters(std::initializer_list<GLenum> names,
                                                         std::initializer_list<GLenum> values){
        parameters.reserve(names.size());

        auto i_value = values.begin();
        for(auto i_name = names.begin(); i_name != names.end() && i_value != values.end(); i_name++){
            parameters.emplace_back(OpenGL3_TextureParameter_Pair{*i_name, *i_value});
        }
    }

    std::vector<OpenGL3_TextureParameters::OpenGL3_TextureParameter_Pair>::const_iterator
    OpenGL3_TextureParameters::begin() const {
        return parameters.begin();
    }

    std::vector<OpenGL3_TextureParameters::OpenGL3_TextureParameter_Pair>::const_iterator
    OpenGL3_TextureParameters::end() const {
        return parameters.end();
    }


    OpenGL3_Texture::OpenGL3_Texture(const GLenum texture_type, const GLenum image_format,
                                     const OpenGL3_TextureParameters& parameters,
                                     const unsigned int texture_width, const unsigned int texture_height,
                                     const GLenum data_format, const GLenum data_type,
                                     const void* data) {
        id = 0;
        glGenTextures(1, &id);
        glBindTexture(texture_type, id);
        glTexImage2D(texture_type, 0, image_format, texture_width, texture_height, 0, data_format, data_type, data);

        for(const auto parameter_pair : parameters){
            glTexParameteri(texture_type, parameter_pair.name, parameter_pair.value);
        }
        bound_type = texture_type;
        width = texture_width;
        height = texture_height;
    }

    OpenGL3_Texture::~OpenGL3_Texture() {
        glDeleteTextures(1, &id);
    }

    void OpenGL3_Texture::make_active_in_slot(const unsigned int slot) {
        //  slot in [0, GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS)
        glActiveTexture(GL_TEXTURE0+slot);
        glBindTexture(bound_type, id);
    }
}