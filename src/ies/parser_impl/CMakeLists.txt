set(parser_impl_SOURCES		common_ies_parser_impl.h common_ies_parser_impl.cpp
				common_string_view_manipulation.h common_string_view_manipulation.cpp
				ies86_parser_impl.h ies86_parser_impl.cpp
				ies91_parser_impl.h ies91_parser_impl.cpp
				ies95_parser_impl.h ies95_parser_impl.cpp
				ies02_parser_impl.h ies02_parser_impl.cpp
				)

list(TRANSFORM parser_impl_SOURCES PREPEND "parser_impl/")
