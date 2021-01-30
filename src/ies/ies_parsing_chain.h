#ifndef REWORKED_PARSER_IES_PARSING_CHAIN_H
#define REWORKED_PARSER_IES_PARSING_CHAIN_H

#include <concepts>
#include <optional>

#include "parser_impl/common_ies_parser_impl.h"

namespace ies {
    template<typename... ParserImplementations>
    class IES_ParsingChain {
    public:

        std::optional<IES_Document> parse(std::string file_name, std::string&& read_data) const {
            std::optional<IES_Document> result;

            bool handled = ((ParserImplementations::can_handle(read_data) &&
                             (result = ParserImplementations::handle(std::move(file_name),
                                                                     std::move(read_data)), true)) || ...);
            if (handled) {
                return result;
            }
            return std::nullopt;
        }
    };
}


#endif //REWORKED_PARSER_IES_PARSING_CHAIN_H
