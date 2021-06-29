#include "opengl3_texture.h"

namespace engine {
    OpenGL3_TextureParametersInts::container::const_iterator OpenGL3_TextureParametersInts::begin() const {
        return parameters.begin();
    }

    OpenGL3_TextureParametersInts::container::const_iterator OpenGL3_TextureParametersInts::end() const {
        return parameters.end();
    }

    OpenGL3_TextureParametersInts::insertion_result OpenGL3_TextureParametersInts::add_parameter(unsigned int name, unsigned int value) {
        return parameters.insert_or_assign(name, value);
    }

    unsigned int OpenGL3_Texture2D::id() const {
        return resource_id;
    }

    OpenGL3_Texture2D::OpenGL3_Texture2D(unsigned int texture_id, unsigned int texture_width,
                                         unsigned int texture_height)
                                         : resource_id{texture_id},
                                            bound_type{GL_TEXTURE_2D},
                                            width{texture_width},
                                            height{texture_height}{}

    OpenGL3_Texture2D::~OpenGL3_Texture2D() {
        glDeleteTextures(1, &resource_id);
    }

    void OpenGL3_Texture2D::bind_to_slot(const unsigned int slot_number) const {
        //  slot in [0, GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS)
        glActiveTexture(GL_TEXTURE0 + slot_number);
        glBindTexture(bound_type, resource_id);
    }

    OpenGL3_Texture1D::~OpenGL3_Texture1D() {
        glDeleteTextures(1, &resource_id);
    }

    void OpenGL3_Texture1D::bind_to_slot(const unsigned int slot_number) const {
        //  slot in [0, GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS)
        glActiveTexture(GL_TEXTURE0 + slot_number);
        glBindTexture(bound_type, resource_id);
    }

    unsigned int OpenGL3_Texture1D::id() const {
        return resource_id;
    }

    OpenGL3_Texture1D::OpenGL3_Texture1D(unsigned int texture_id, int texture_dimension)
    : resource_id{texture_id},
        bound_type{GL_TEXTURE_1D},
        dimension{texture_dimension} {}

    OpenGL3_Texture2D_Builder&& OpenGL3_Texture2D_Builder::with_size(int tex_width, int tex_height)&& {
        this->width = tex_width;
        this->height = tex_height;
        return std::move(*this);
    }

    OpenGL3_Texture2D_Builder&& OpenGL3_Texture2D_Builder::using_border_color(const std::array<float, 4>& color)&& {
        border_color = color;
        return std::move(*this);
    }

    OpenGL3_Texture2D_Builder&& OpenGL3_Texture2D_Builder::using_clamping_to_borders()&& {
        parameters.add_parameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        parameters.add_parameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        return std::move(*this);
    }

    OpenGL3_Texture2D_Builder&& OpenGL3_Texture2D_Builder::using_clamping_to_edge()&& {
        parameters.add_parameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        parameters.add_parameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        return std::move(*this);
    }

    std::unique_ptr<OpenGL3_Texture2D> OpenGL3_Texture2D_Builder::as_resource() {
        glGenTextures(1, &id);
        glBindTexture(GL_TEXTURE_2D, id);
        glTexImage2D(GL_TEXTURE_2D, 0, texture_format, width, height, 0, texture_data_format, data_type, nullptr);
        for(auto&& parameter_pair : parameters){
            glTexParameteri(GL_TEXTURE_2D, parameter_pair.first, parameter_pair.second);
        }
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border_color.data());
        return std::unique_ptr<OpenGL3_Texture2D>(new OpenGL3_Texture2D(id, width, height));
    }

    std::unique_ptr<OpenGL3_Texture2D> OpenGL3_Texture2D_Builder::as_resource_with_data(const void* data) {
        glGenTextures(1, &id);
        glBindTexture(GL_TEXTURE_2D, id);
        glTexImage2D(GL_TEXTURE_2D, 0, texture_format, width, height, 0, texture_data_format, data_type, data);
        for(auto&& parameter_pair : parameters){
            glTexParameteri(GL_TEXTURE_2D, parameter_pair.first, parameter_pair.second);
        }
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border_color.data());
        return std::unique_ptr<OpenGL3_Texture2D>(new OpenGL3_Texture2D(id, width, height));
    }

    OpenGL3_Texture1D_Builder&& OpenGL3_Texture1D_Builder::with_size(int size)&& {
        this->dimension = size;
        return std::move(*this);
    }

    OpenGL3_Texture1D_Builder&& OpenGL3_Texture1D_Builder::using_clamping_to_borders()&& {
        parameters.add_parameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        return std::move(*this);
    }

    OpenGL3_Texture1D_Builder&& OpenGL3_Texture1D_Builder::using_clamping_to_edge()&& {
        parameters.add_parameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        return std::move(*this);
    }

    std::unique_ptr<OpenGL3_Texture1D> OpenGL3_Texture1D_Builder::as_resource() {
        glGenTextures(1, &id);
        glBindTexture(GL_TEXTURE_1D, id);
        glTexImage1D(GL_TEXTURE_1D, 0, texture_format, dimension, 0, texture_data_format, data_type, nullptr);
        for(auto&& parameter_pair : parameters){
            glTexParameteri(GL_TEXTURE_1D, parameter_pair.first, parameter_pair.second);
        }
        return std::unique_ptr<OpenGL3_Texture1D>(new OpenGL3_Texture1D(id, dimension));
    }

    std::unique_ptr<OpenGL3_Texture1D> OpenGL3_Texture1D_Builder::as_resource_with_data(const void* data) {
        glGenTextures(1, &id);
        glBindTexture(GL_TEXTURE_1D, id);
        glTexImage1D(GL_TEXTURE_1D, 0, texture_format, dimension, 0, texture_data_format, data_type, data);
        for(auto&& parameter_pair : parameters){
            glTexParameteri(GL_TEXTURE_1D, parameter_pair.first, parameter_pair.second);
        }
        return std::unique_ptr<OpenGL3_Texture1D>(new OpenGL3_Texture1D(id, dimension));
    }
}