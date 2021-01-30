set(ies_SOURCES		ies_parser.h
			ies_parsing.h
			ies_parsing_chain.h
			ies_printing.h
			ies_document.h ies_document.cpp
			ies_label_data.h ies_label_data.cpp
			ies_data.h ies_data.cpp
			ies_default_parser.h ies_default_parser.cpp
			)

include("${CMAKE_CURRENT_LIST_DIR}/adapter/CMakeLists.txt")
include("${CMAKE_CURRENT_LIST_DIR}/parser_impl/CMakeLists.txt")

list(APPEND ies_SOURCES ${adapter_SOURCES})
list(APPEND ies_SOURCES ${parser_impl_SOURCES})

list(TRANSFORM ies_SOURCES PREPEND "ies/")
list(APPEND cornell_box_SOURCES ${ies_SOURCES})