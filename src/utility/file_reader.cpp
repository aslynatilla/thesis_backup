#include "file_reader.h"

std::string files::read_file(const std::filesystem::path& normalized_path) {
    std::ifstream file_from_path;
    file_from_path.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try {
        file_from_path.open(normalized_path.string());
        std::stringstream string_converter;
        string_converter << file_from_path.rdbuf();
        file_from_path.close();
        return string_converter.str();
    }
    catch (std::ios_base::failure& fail) {
        std::cerr << "Failure in reading a file. An empty string will be returned in its stead.\n"
                  << fail.code().message() << "\n" << fail.what() << "\n";
        return std::string();
    }
}

std::filesystem::path files::make_path_absolute(std::string&& input_path) {
    namespace fs = std::filesystem;
    fs::path path_from_string(std::move(input_path));
    return fs::absolute(path_from_string.lexically_normal());
}
