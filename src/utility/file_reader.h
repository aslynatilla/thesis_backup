#ifndef FILE_READER_H
#define FILE_READER_H

#include <fstream>
#include <filesystem>
#include <iostream>
#include <sstream>
#include <string>

namespace files {
    [[nodiscard]] std::string read_file(const std::filesystem::path& normalized_path);

    [[nodiscard]] std::filesystem::path make_path_absolute(std::string&& input_path);
}

#endif //FILE_READER_H
