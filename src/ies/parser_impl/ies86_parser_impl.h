#ifndef IES86_PARSER_IMPL_H
#define IES86_PARSER_IMPL_H

#include "common_ies_parser_impl.h"
#include "common_string_view_manipulation.h"

namespace ies::parser_impl {
    class IES86_ParserImpl {
    public:
        static const char label_delimiter = ':';

        static bool can_handle(const std::string_view& ies_data);

        static std::optional<IES_Document> handle(const std::string& filename, std::string&& ies_data);

        static bool contains_colon(const std::string_view& line);
    };

}


#endif //IES86_PARSER_IMPL_H
