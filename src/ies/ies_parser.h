#ifndef REWORKED_PARSER_IES_PARSER_H
#define REWORKED_PARSER_IES_PARSER_H

#include "ies_parsing_chain.h"

namespace ies {
    template<typename... ParserImplementations>
    class IES_Parser {
    public:
        IES_ParsingChain<ParserImplementations...> parsing_impl;

        std::optional<IES_Document> parse(std::string file_name, std::string&& read_data) const {
            return parsing_impl.parse(std::move(file_name), std::move(read_data));
        }
    };

}

#endif //REWORKED_PARSER_IES_PARSER_H
