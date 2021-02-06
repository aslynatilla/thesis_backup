#ifndef SHADER_LOADING_H
#define SHADER_LOADING_H

#include "shader.h"
#include "../../utility/file_reader.h"

namespace engine::shader{
    std::shared_ptr<Shader> create_shader_from(const std::string& vertex_path_as_string,
                                               const std::string& fragment_path_as_string);
    std::shared_ptr<Shader> create_shader_from(const std::string& vertex_path_as_string,
                                               const std::string& fragment_path_as_string,
                                               const std::string& geometry_path_as_string);
};


#endif //SHADER_LOADING_H
