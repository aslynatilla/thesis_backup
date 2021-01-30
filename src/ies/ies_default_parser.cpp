#include "ies_default_parser.h"

namespace ies {

    IES_Document IES_Default_Parser::parse(const std::string& filename, std::string&& file_data) const {
        auto maybe_document = parser.parse(filename, std::move(file_data));

        if (maybe_document == std::nullopt) {
            throw std::domain_error(filename + " could not be parsed.\n");
        }

        return IES_Document(std::move(maybe_document.value()));
    }
}