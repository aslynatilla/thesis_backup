#include "shader_loading.h"

namespace engine::shader{

    std::shared_ptr<Shader> create_shader_from(const std::string& vertex_path_as_string,
                                               const std::string& fragment_path_as_string){
        const auto vertex_path = files::make_path_absolute(std::string(vertex_path_as_string));
        const auto fragment_path = files::make_path_absolute(std::string(fragment_path_as_string));
        auto vertex_source = files::read_file(vertex_path);
        auto fragment_source = files::read_file(fragment_path);
        const std::string vertex_filename(vertex_path.filename().string());
        const std::string fragment_filename(fragment_path.filename().string());
        return std::make_shared<Shader>(std::move(vertex_source),
                                        std::move(fragment_source),
                                        std::string_view(vertex_filename),
                                        std::string_view(fragment_filename));
    }

    std::shared_ptr<Shader>
    shader::create_shader_from(const std::string& vertex_path_as_string, const std::string& fragment_path_as_string,
                               const std::string& geometry_path_as_string) {
        const auto vertex_path = files::make_path_absolute(std::string(vertex_path_as_string));
        const auto fragment_path = files::make_path_absolute(std::string(fragment_path_as_string));
        const auto geometry_path = files::make_path_absolute(std::string(geometry_path_as_string));
        auto vertex_source = files::read_file(vertex_path);
        auto fragment_source = files::read_file(fragment_path);
        auto geometry_source = files::read_file(geometry_path);
        const std::string vertex_filename(vertex_path.filename().string());
        const std::string fragment_filename(fragment_path.filename().string());
        const std::string geometry_filename(geometry_path.filename().string());
        return std::make_shared<Shader>(std::move(vertex_source),
                                        std::move(geometry_source),
                                        std::move(fragment_source),
                                        std::string_view(vertex_filename),
                                        std::string_view(geometry_filename),
                                        std::string_view(fragment_filename));
    }
}