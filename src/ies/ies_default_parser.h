#ifndef IES_DEFAULT_PARSER_H
#define IES_DEFAULT_PARSER_H

#include "ies_parsing.h"

namespace ies {
    class IES_Default_Parser {
    public:
        IES_Default_Parser() = default;

        IES_Default_Parser(const IES_Parser<parser_impl::IES02_ParserImpl,
                parser_impl::IES95_ParserImpl,
                parser_impl::IES91_ParserImpl,
                parser_impl::IES86_ParserImpl> parser) = delete;

        IES_Document parse(const std::string& filename, std::string&& file_data) const;

    private:
        IES_Parser<parser_impl::IES02_ParserImpl,
                parser_impl::IES95_ParserImpl,
                parser_impl::IES91_ParserImpl,
                parser_impl::IES86_ParserImpl> parser;
    };
}


#endif //IES_DEFAULT_PARSER_H
