#include "opengl3_texture.h"

namespace engine{

    OpenGL3_TextureParameters::OpenGL3_TextureParameters(std::initializer_list<GLenum> names,
                                                         std::initializer_list<GLenum> values){
        parameters.reserve(names.size());

        auto i_value = values.begin();
        for(auto i_name = names.begin(); i_name != names.end() && i_value != values.end(); i_name++, i_value++){
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


    OpenGL3_Texture2D::OpenGL3_Texture2D(const GLenum image_format,
                                         const OpenGL3_TextureParameters& parameters,
                                         const unsigned int texture_width, const unsigned int texture_height,
                                         const GLenum data_format, const GLenum data_type,
                                         const void* data) {
        id = 0;
        glGenTextures(1, &id);
        glBindTexture(GL_TEXTURE_2D, id);
        glTexImage2D(GL_TEXTURE_2D, 0, image_format, texture_width, texture_height, 0, data_format, data_type, data);
        for(const auto parameter_pair : parameters){
            glTexParameteri(GL_TEXTURE_2D, parameter_pair.name, parameter_pair.value);
        }
        bound_type = GL_TEXTURE_2D;
        width = texture_width;
        height = texture_height;
    }

    OpenGL3_Texture2D::~OpenGL3_Texture2D() {
        glDeleteTextures(1, &id);
    }

    void OpenGL3_Texture2D::bind_to_slot(const unsigned int slot_number) {
        //  slot in [0, GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS)
        glActiveTexture(GL_TEXTURE0 + slot_number);
        glBindTexture(bound_type, id);
    }

    std::unique_ptr<OpenGL3_Texture2D>
    OpenGL3_Texture2D::create_default_texture_linear(const GLenum texture_format,
                                                     const GLenum data_format,
                                                     const unsigned int tex_width,
                                                     const unsigned int tex_height) {
        return std::make_unique<OpenGL3_Texture2D>(texture_format,
                                                   OpenGL3_TextureParameters(
                                                           {GL_TEXTURE_MIN_FILTER, GL_TEXTURE_MAG_FILTER,
                                                            GL_TEXTURE_WRAP_S, GL_TEXTURE_WRAP_T},
                                                           {GL_LINEAR, GL_LINEAR,
                                                            GL_CLAMP_TO_BORDER, GL_CLAMP_TO_BORDER}),
                                                   tex_width, tex_height,
                                                   data_format, GL_FLOAT, nullptr);
    }

    std::unique_ptr<OpenGL3_Texture2D>
    OpenGL3_Texture2D::create_default_texture_nearest(const GLenum texture_format,
                                                      const GLenum data_format,
                                                      const unsigned int tex_width,
                                                      const unsigned int tex_height) {
        return std::make_unique<OpenGL3_Texture2D>(texture_format,
                                                   OpenGL3_TextureParameters(
                                                           {GL_TEXTURE_MIN_FILTER, GL_TEXTURE_MAG_FILTER,
                                                            GL_TEXTURE_WRAP_S, GL_TEXTURE_WRAP_T},
                                                           {GL_NEAREST, GL_NEAREST,
                                                            GL_CLAMP_TO_BORDER, GL_CLAMP_TO_BORDER}),
                                                   tex_width, tex_height,
                                                   data_format, GL_FLOAT, nullptr);    }


    OpenGL3_Texture1D::OpenGL3_Texture1D(const GLenum image_format, const OpenGL3_TextureParameters& parameters,
                                         const unsigned int texture_width, const GLenum data_format,
                                         const GLenum data_type, const void* data) {
        id = 0;
        glGenTextures(1, &id);
        glBindTexture(GL_TEXTURE_1D, id);
        glTexImage1D(GL_TEXTURE_1D, 0, image_format, texture_width, 0, data_format, data_type, data);
        [[maybe_unused]] auto error = glGetError() != 0;
        for(const auto parameter_pair : parameters){
            glTexParameteri(GL_TEXTURE_1D, parameter_pair.name, parameter_pair.value);
        }
        bound_type = GL_TEXTURE_1D;
        width = texture_width;
    }

    OpenGL3_Texture1D::~OpenGL3_Texture1D() {
        glDeleteTextures(1, &id);
    }

    void OpenGL3_Texture1D::bind_to_slot(const unsigned int slot_number) {
        //  slot in [0, GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS)
        glActiveTexture(GL_TEXTURE0 + slot_number);
        glBindTexture(bound_type, id);
    }
}