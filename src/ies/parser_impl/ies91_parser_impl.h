#ifndef IES91_PARSER_IMPL_H
#define IES91_PARSER_IMPL_H

#include "common_ies_parser_impl.h"
#include "common_string_view_manipulation.h"

namespace ies::parser_impl {
    class IES91_ParserImpl {
    public:
        static const char label_delimiter = ']';

        static bool can_handle(const std::string_view& ies_data);

        static std::optional<IES_Document> handle(const std::string& filename, std::string&& ies_data);

        static bool no_more_entry_lines(const std::string_view& line);
    };
}


#endif //IES91_PARSER_IMPL_H
