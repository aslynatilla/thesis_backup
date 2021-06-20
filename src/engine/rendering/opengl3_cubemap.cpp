#include "opengl3_cubemap.h"

namespace engine{
    OpenGL3_Cubemap::~OpenGL3_Cubemap() {
        glDeleteTextures(1, &resource_id);
    }

    unsigned int OpenGL3_Cubemap::id() const {
        return resource_id;
    }

    void OpenGL3_Cubemap::bind_to_slot(const unsigned int slot_number) const {
        glActiveTexture(GL_TEXTURE0 + slot_number);
        glBindTexture(bound_type, resource_id);
    }

    OpenGL3_Cubemap::OpenGL3_Cubemap(unsigned int texture_id, unsigned int texture_width, unsigned int texture_height)
    : resource_id{texture_id},
    bound_type{GL_TEXTURE_CUBE_MAP},
    width{texture_width},
    height{texture_height}{}

    OpenGL3_Cubemap_Builder&& OpenGL3_Cubemap_Builder::with_size(int tex_width, int tex_height)&& {
        this->width = tex_width;
        this->height = tex_height;
        return std::move(*this);
    }

    OpenGL3_Cubemap_Builder&& OpenGL3_Cubemap_Builder::using_clamping_to_borders()&& {
        parameters.add_parameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        parameters.add_parameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        return std::move(*this);
    }

    OpenGL3_Cubemap_Builder&& OpenGL3_Cubemap_Builder::using_clamping_to_edge()&& {
        parameters.add_parameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        parameters.add_parameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        return std::move(*this);
    }

    std::unique_ptr<OpenGL3_Cubemap> OpenGL3_Cubemap_Builder::as_resource() {
        glGenTextures(1, &id);
        glBindTexture(GL_TEXTURE_CUBE_MAP, id);
        for(auto i = 0; i < 6; ++i){
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, texture_format, width, height, 0, texture_data_format, data_type,
                         nullptr);
        }
        for(const auto parameter_pair : parameters){
            glTexParameteri(GL_TEXTURE_CUBE_MAP, parameter_pair.first, parameter_pair.second);
        }
        return std::unique_ptr<OpenGL3_Cubemap>(new OpenGL3_Cubemap(id, width, height));
    }
}

